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

void	EpollServer::initMultiplexer( void )
{
	this->_epoll_fd = epoll_create(1);
	if (this->_epoll_fd == -1)
		throw runtimeEpollServerException("Error\nEpoll init.");
}

void	EpollServer::addSocketsToMultiplexer( void )
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

void	EpollServer::run( void )
{
	while (1)
	{
		int fd_count = epoll_wait(this->_epoll_fd, this->_active_events, MAX_EVENTS, -1);
		if (fd_count == -1)
		{
			if (errno == EINTR)
				continue;
			throw runtimeEpollServerException((std::string("Error fatal:\n") + strerror(errno)).c_str());
		}
		for (int i = 0; i < fd_count; ++i)
		{
			int current_fd = this->_active_events[i].data.fd;
			if (this->fdMatch(current_fd))
			{
				if (!this->addNewClient(current_fd))
					continue;
			}
			else if (this->_active_events[i].events & EPOLLIN)
			{
				this->readRequest(current_fd);
			}
			else if (this->_active_events[i].events & EPOLLOUT)
			{
				this->sendResponse(current_fd);
			}
		}
	}
}

void	EpollServer::addClientToMultiplexer( int fd )
{
	struct epoll_event client_ev;
	client_ev.events = EPOLLIN;
	client_ev.data.fd = fd;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, fd, &client_ev) == -1)
		throw runtimeEpollServerException("Error\nepoll_ctl()");
}

void	EpollServer::removeFdFromMultiplexer( int fd )
{
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void	EpollServer::watchForRead( int fd )
{
	struct epoll_event mod_ev;
	mod_ev.events = EPOLLIN;
	mod_ev.data.fd = fd;
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, fd, &mod_ev);
}

void	EpollServer::watchForWrite( int fd )
{
	struct epoll_event mod_ev;
	mod_ev.events = EPOLLOUT;
	mod_ev.data.fd = fd;
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, fd, &mod_ev);
}

void	EpollServer::closeEpoll( void )
{
	if (this->_epoll_fd != -1)
		close(this->_epoll_fd);
	this->_epoll_fd = -1;
}

#else

void	EpollServer::initMultiplexer( void )
{
	throw runtimeEpollServerException("Epoll is not available on this platform.");
}

void	EpollServer::addSocketsToMultiplexer( void ) {}

void	EpollServer::run( void )
{
	throw runtimeEpollServerException("Epoll is not available on this platform.");
}

void	EpollServer::addClientToMultiplexer( int ) {}

void	EpollServer::removeFdFromMultiplexer( int ) {}

void	EpollServer::watchForRead( int ) {}

void	EpollServer::watchForWrite( int ) {}

#endif
