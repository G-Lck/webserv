#include "../../includes/EpollServer.hpp"

// --------------- Orthodoxy ---------------

EpollServer::EpollServer()
{
	#ifdef __linux__
	this->_epoll_fd = -1;
	memset(&_event, 0, sizeof(_event));
	memset(_active_events, 0, sizeof(_active_events));
	#endif
}

EpollServer::~EpollServer()
{
	#ifdef __linux__
	closeEpoll();
	#endif
}

// --------------- Exceptions ---------------

EpollServer::runtimeEpollServerException::runtimeEpollServerException(const char* message)
	: std::runtime_error(message) {}

#ifdef __linux__

// --------------- Set up Epoll ---------------

/// @brief Initialize epoll, store fd from epoll in the EpollServer member variable
void	EpollServer::initMultiplexer(void)
{
	this->_epoll_fd = epoll_create(1);
	if (this->_epoll_fd == -1)
		throw runtimeEpollServerException("Error\nEpoll init.");
}

/// @brief	Iterates though the map of fd:Sockets in the EpollServer
///			Store in epoll the fd of that Socket, init the event to EPOLLIN
///			And with epoll_ctl using EPOLL_CTL_ADD flag we add a 
///			new entry inside epoll for that fd (or Socket):
///			Inside the struct: We are linking &this->_event with that Socket
/// @param  
void	EpollServer::addSocketsToMultiplexer(void)
{
	std::map<int, Socket*>::iterator it = this->_sockets.begin();
	while (it != this->_sockets.end())
	{
		this->_event.events = EPOLLIN;
		this->_event.data.fd = it->second->getFd();
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, it->second->getFd(), &this->_event) == -1)
			throw runtimeEpollServerException("Error\nepoll_ctl()");
		++it;
	}
}

/// @brief	54. Check if at least one socket connected
///			57. Start main loop
///			60. We call epoll_wait - waits for an event to happen in the first parameter fd
///					* this->_epoll_fd: The epoll fd
///					* this->_active_events: Here epoll_wait stores the struct epoll_event of the event happend for that fd
///					* MAX_EVENTS: max number of events
///					* -1: This is witing time, when set to -1 is no waiting time
/// @param  
void	EpollServer::run(void)
{
	if (this->_sockets.empty())
		throw runtimeEpollServerException("Error\nNo sockets to serve.");
	while (1)
	{
		int fd_count = epoll_wait(this->_epoll_fd, this->_active_events, MAX_EVENTS, SERVER_REFRESH_TIME);
		if (fd_count == -1)
		{
			if (errno == EINTR)
				continue;
			throw runtimeEpollServerException((std::string("Error fatal:\n") + strerror(errno)).c_str());
		}
		if (fd_count == 0) //+ Case for no active events
		{
			// timeout expired
			monitorClients();
			// monitorCGI();
			continue;
		}
		// Here we are itearing on all the events to run the right case for each fd
		for (int i = 0; i < fd_count; ++i)
		{
			int current_fd = this->_active_events[i].data.fd;
			// Cases:
			if (this->fdIsCgi(current_fd))
			{
				//handleCgiEvent(current_fd, this->_active_events[i].events);
			}
			// current_fd matches a listening socket: a new client
			// connection is waiting to be accepted
			else if (this->fdMatch(current_fd))
			{
				if (!this->addNewClient(current_fd))
					continue;
			}
			// current_fd is an existing client, and it sent data (EPOLLIN):
			// read the incoming request
			else if (this->_active_events[i].events & EPOLLIN)
			{
				this->refreshClientTime(current_fd);
				this->readRequest(current_fd);
			}
			// current_fd is an existing client, and it's ready to receive
			// data (EPOLLOUT): send the pending response
			else if (this->_active_events[i].events & EPOLLOUT)
			{
				this->refreshClientTime(current_fd);
				this->sendResponse(current_fd);
			}
		}
		monitorClients(); //+ Check again all, case for when an active event
		// monitorCGI();
	}
}

/// @brief	This function will try to add an fd to the client_ev inside epoll
///			if epoll_ctl call fails, an error will be logged.
/// @param fd The fd to add
/// @return True if it managed to add the client correctly
bool	EpollServer::addClientToMultiplexer( int fd )
{
	struct epoll_event client_ev;
	client_ev.events = EPOLLIN;
	client_ev.data.fd = fd;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, fd, &client_ev) == -1)
	{
		writeLog(getEpollCtlErrorStr(errno), ERROR_INFO);
		return false;
	}
	return true;
}

/// @brief	This function will try to remove an fd from epoll, if the fd
///			passed is not found, then an error will log. Execution continues.
/// @param fd The fd to remove
void	EpollServer::removeFdFromMultiplexer( int fd )
{
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
		writeLog(getEpollCtlErrorStr(errno), ERROR_INFO);
}

/// @brief	Function to change to Read mode that fd inside epoll list of fd
///			Errors will be logged if epoll_ctl fails and we continue execution.
void	EpollServer::watchForRead( int fd )
{
	struct epoll_event mod_ev;
	mod_ev.events = EPOLLIN;
	mod_ev.data.fd = fd;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, fd, &mod_ev) == -1)
		writeLog(getEpollCtlErrorStr(errno), ERROR_INFO);
}

/// @brief	Function to change to Write mode that fd inside epoll list of fd
///			Errors will be logged if epoll_ctl fails and we continue execution.
void	EpollServer::watchForWrite( int fd )
{
	struct epoll_event mod_ev;
	mod_ev.events = EPOLLOUT;
	mod_ev.data.fd = fd;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, fd, &mod_ev) == -1)
		writeLog(getEpollCtlErrorStr(errno), ERROR_INFO);
}

void	EpollServer::closeEpoll(void)
{
	if (this->_epoll_fd != -1)
		close(this->_epoll_fd);
	this->_epoll_fd = -1;
}

void	EpollServer::handleCgiEvent(int fd, uint32_t epoll_event)
{
    // handler = lookup CgiHandler* by fd   // from AServer's cgi map
    // if handler == NULL:
    //     return  // shouldn't happen, safety check

    // if events & EPOLLOUT and fd == handler->getStdinFd():
    //     write next chunk of handler's write buffer to fd
    //     if all body written:
    //         close stdin fd, remove from epoll, mark handler side done

    // else if events & EPOLLIN and fd == handler->getStdoutFd():
    //     read available data from fd into handler's read buffer
    //     if read returns 0 (EOF):
    //         mark cgi output finished
    //         close stdout fd, remove from epoll
    //         waitpid on handler's pid (non-blocking check or already reaped by monitorCgi)
    //         build HTTP response from read buffer
    //         attach response to handler's _client
    //         re-arm client fd for EPOLLOUT
    //         cleanup: erase fd(s) from AServer's cgi map, delete handler
}
#else

void	EpollServer::initMultiplexer(void)
{
	throw runtimeEpollServerException("Epoll is not available on this platform.");
}

void	EpollServer::addSocketsToMultiplexer(void) {}

void	EpollServer::run(void)
{
	throw runtimeEpollServerException("Epoll is not available on this platform.");
}

bool	EpollServer::addClientToMultiplexer( int ) {return true;}

void	EpollServer::removeFdFromMultiplexer( int ) {}

void	EpollServer::watchForRead( int ) {}

void	EpollServer::watchForWrite( int ) {}

#endif
