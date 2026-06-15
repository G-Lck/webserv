#include "../../includes/WebServ.hpp"

// --------------- Orthodoxy ---------------

Server::Server() : _epoll_fd(-1) { memset(&_event, 0, sizeof(_event));  memset(_active_events, 0, sizeof(_active_events)); }

Server::~Server() { closeSockets(); closeEpoll(); }

// --------------- Exceptions ---------------

Server::runtimeServerException::runtimeServerException(const char* message) : std::runtime_error(message) {}

// --------------- Set up Epoll ---------------

/// @brief Add a single Socket to the _socket Vector.
/// @param S The socket to add
void	Server::addSocket( Socket *S ) { this->_sockets.push_back(S); }

/// @brief	Creates the epoll object.
///			Stores the fd into _epoll_fd.
///	@note	Think of it as a list you're going to fill with fds to watch — Close it when you're done.
/// @throw	runtimeServerException() if epoll_create(1) fails.
void	Server::epollInit( void )
{
	this->_epoll_fd = epoll_create(1);
    if (this->_epoll_fd == -1)
		throw runtimeServerException("Error\nEpol init.");
}

/// @brief  Add the sockets to epoll to watch for new connections
/// @throw	runtimeServerException() if epoll_ctl() fails.
void    Server::epollAddSockets( void )
{
	std::vector<Socket*>::iterator it = this->_sockets.begin();

	while (it != this->_sockets.end())
	{
		this->_event.events = EPOLLIN; 
		this->_event.data.fd = (*it)->getFd(); 
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, (*it)->getFd(), &this->_event) == -1)
			throw runtimeServerException("Error\nepoll_ctl()");
		++it;
	}
}

/// @brief Helper to close at the end all Sockets FD's
void	Server::closeSockets( void )
{
	std::vector<Socket*>::iterator it = this->_sockets.begin();

	while (it != this->_sockets.end())
	{
		(*it)->closeFd();
		++it;
	}
}

/// @brief Close Epoll FD
void	Server::closeEpoll( void ) { close(this->_epoll_fd); }

// --------------- Main Loop ---------------

/// @brief 
///	@param active_events Is the array to hold the events that epoll_wait creates for us
///	@param fd_count Used to store how many fds are ready. The kernel fills active_events with only the ready ones
///	@param EINTR If the OS just interrupted us with a signal, ignore and continue
/// @details Scenario A: The Main Socket is Ready (New Connection) | We call addNewClient()
/// 		 Scenario B: A Client Socket is Ready to READ (EPOLLIN) | We call readRequest()
///			 Scenario C: A Client Socket is Ready to WRITE (EPOLLOUT) | We call sendResponse()

void	Server::initServer( void )
{
	struct epoll_event active_events[MAX_EVENTS];

	while (1)
	{
		int fd_count = epoll_wait(this->_epoll_fd, active_events, MAX_EVENTS, -1);
		
		if (fd_count == -1)
		{
			if (errno == EINTR)
				continue;
			throw runtimeServerException((std::string("Error fatal:\n") + strerror(errno)).c_str());
		}
		for (int i = 0; i < fd_count; ++i)
		{
			int current_fd = active_events[i].data.fd;
			if (this->fdMatch(current_fd))
			{
				if (!this->addNewClient(current_fd))
					continue ;
			}
			else if (active_events[i].events & EPOLLIN)
			{
				this->readRequest(current_fd);
			}
			else if (active_events[i].events & EPOLLOUT)
			{
				this->sendResponse(current_fd);
			}
		}
	}
}

// --------------- Handle Cases ---------------


//* ------- Scenario A -------

/// @brief Iterates through sockets until finding a matching fd
/// @param curr Fd to match
/// @return True if found
bool	Server::fdMatch( int curr )
{
	std::vector<Socket*>::iterator it = this->_sockets.begin();

	while( it != this->_sockets.end())
	{
		if ((*it)->getFd() == curr)
			return ( true );
		++it;
	}
	return ( false );
}

/// @brief	Creates a new client and adds it to epoll
/// @param curr_socket_fd The current socket (Server::initServer() iteration) file descriptor.
///	@details calls accept() to actually get a new socket descriptor to use
///			 for subsequent communication with the newly connected client.
///			 Current socket remains. Will be used for further accept() calls as they come in.
///			 The client_ev epoll is localy allocated, set up the correct values and then
///			 the client fd and data is added to epoll.
///			 The epoll_event is then destroyed localy.
///			 The fd of the new client is stored inside a member vector for later safe checks.
bool	Server::addNewClient( int curr_socket_fd )
{
	struct sockaddr_in  addr_client;
	socklen_t           addr_size = sizeof(addr_client);

	int fd_client = accept(curr_socket_fd, (struct sockaddr *)&addr_client, &addr_size);
	if (fd_client == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (false);
		std::cout << "accept() failed: " << strerror(errno) << std::endl;
		return (false);	
	}
	fcntl(fd_client, F_SETFL, O_NONBLOCK);
	struct epoll_event client_ev;
	client_ev.events = EPOLLIN;
	client_ev.data.fd = fd_client;
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, fd_client, &client_ev);
	return (true);
}

//* ------- Scenario B -------

/// @brief	Function to read a request. It will internally call handleCompleteRequest() to
///			excecute that request, unless no bytes are read (or err) by recv, in that case, we call
///			clientDisconect() to clear the data.
/// @param fd The current client fd for the request being handled
void	Server::readRequest( int fd )
{
	char buffer[4096];
	int bytes_read = recv(fd, buffer, sizeof(buffer), 0);

	if (bytes_read <= 0)
		this->clientDisconnect(fd);
	else
	{
		this->_client_buffers[fd].append(buffer, bytes_read);
		while (request_is_complete(this->_client_buffers, fd))
			this->handleCompleteRequest(fd);
	}
}

/// @brief Removes client from epoll, also cleaning the buffer and response vectors
/// @param fd The client fd to be removed.
///	@details ~ALWAYS remove from epoll before closing the FD
void	Server::clientDisconnect( int fd )
{
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	this->_client_buffers.erase(fd);
	this->_client_responses.erase(fd);
}

/// @brief  Extracts and processes a single, complete HTTP request from a client's buffer.
///         Appends the generated HTTP response to the client_response vector and switches the socket
///         to EPOLLOUT so the server knows it is ready to send data.
/// @param fd The client socket file descriptor.
void    Server::handleCompleteRequest( int fd )
{
    //+ 1. Isolate the first complete request from the persistent string (handles pipelining)
    std::string single_request = extract_request(this->_client_buffers, fd);
    
    //+ 2. Parse the HTTP, find the file/CGI, and build the full string response
    std::string single_response = process_and_build_response(single_request);
            
    //+ 3. Push the response to the outbound waiting line
    this->_client_responses[fd].append(single_response);
    
    //+ 4. Delete only the parsed request, leaving any leftover bytes for the next cycle
    erase_request_from_buffer(this->_client_buffers, fd);
    
    //+ 5. Wake up epoll_wait and tell it we want to write to this client
    if (!this->_client_responses[fd].empty())
        this->setEpollInOut(fd, EPOLLOUT);
}

/// @brief If a response is generated, modify epoll to watch for EPOLLOUT or EPOLLIN
/// @param fd The file descriptor of the socket/client that we want to change
///	@param flag Chose EPOLLOUT or EPOLLIN
///	@param EPOLL_CTL_MOD stands for MODIFY
///	@details Even tho we are crating a new epoll_event, by seting this flag in the
///			 epoll_ctl call, it means that we are not adding a new event but modifying
///			 those particual values.
///			 "Find the exact single entry you already have for fd, and overwrite its instructions."
void	Server::setEpollInOut( int fd, int flag )
{
	struct epoll_event mod_ev;
	mod_ev.events = flag; 
	mod_ev.data.fd = fd;
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, fd, &mod_ev);
}

//* ------- Scenario C -------

void	Server::sendResponse( int fd )
{
    int sent_bytes = send(fd, this->_client_responses[fd].c_str(), this->_client_responses[fd].length(), 0);

    if (sent_bytes <= 0)
    {
        this->clientDisconnect(fd);
        return ;
    }

    this->_client_responses[fd].erase(0, sent_bytes);   
    if (this->_client_responses[fd].empty())
        this->setEpollInOut(fd, EPOLLIN);
}
