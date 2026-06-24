#include "../../includes/Server.hpp"

// --------------- Orthodoxy ---------------

Server::Server() : _epoll_fd(-1) { memset(&_event, 0, sizeof(_event));  memset(_active_events, 0, sizeof(_active_events)); }

Server::~Server() { closeSockets(); closeEpoll(); }

// --------------- Exceptions ---------------

Server::runtimeServerException::runtimeServerException(const char* message) : std::runtime_error(message) {}

// --------------- Set up Config ---------------

/// @brief Initializes physical sockets and sets up the virtual host routing maps.
/// @details	Loops through all server blocks to grab their host:port combos. 
/// 			To avoid bind() crashes, we make sure to only create one physical socket 
///				per unique combination (of host+port). While doing this, we also fill the _virtualServers 
///				and _fd_to_route maps so we can quickly route incoming HTTP requests 
///				to the correct server block later during the epoll loop.
/// @param config The GlobalConfig object holding all our parsed server blueprints.
void	Server::configServer( GlobalConfig &config )
{
	t_port_host	 used_ports;
	for (int i = 0; i < (int)config.serverCount(); i++)
	{
		const ServerConfig	&ServConf = config.getServers(i);
		const t_port_host	ports = ServConf.getListen();

		for (int j = 0; j < (int)ports.size(); j++)
		{
			t_port_host::iterator it;
			it = std::find(used_ports.begin(), used_ports.end(), ports[j]);
			if (it == used_ports.end())
			{
				Socket *newSocket = new Socket;
				newSocket->makeSocket(ports[j].second, ports[j].first);
				this->_fd_to_route[newSocket->getFd()] = ports[j];
				used_ports.push_back(ports[j]);
				this->addSocket(newSocket);
			}
			this->_virtualServers[ports[j]].push_back(ServConf);
		}
	}
}

/// @brief Add a single Socket to the _socket Vector.
/// @param S The socket to add
void	Server::addSocket( Socket *S ) { this->_sockets.insert(std::make_pair(S->getFd(), S)); }

// --------------- Set up Epoll ---------------

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
	std::map<int, Socket*>::iterator it = this->_sockets.begin();

	while (it != this->_sockets.end())
	{
		this->_event.events = EPOLLIN; 
		this->_event.data.fd = it->second->getFd(); 
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, it->second->getFd(), &this->_event) == -1)
			throw runtimeServerException("Error\nepoll_ctl()");
		++it;
	}
}

/// @brief Helper to close at the end all Sockets FD's
void	Server::closeSockets( void )
{
	std::map<int, Socket*>::iterator it = this->_sockets.begin();

	while (it != this->_sockets.end())
	{
		it->second->closeFd();
		delete it->second;
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
bool    Server::fdMatch( int curr )
{
    return (this->_sockets.find(curr) != this->_sockets.end());
}

/// @brief	Creates a new client and adds it to epoll
/// @param curr_socket_fd The current socket (Server::initServer() iteration) file descriptor.
///	@details calls accept() to actually get a new socket descriptor to use
///			 for subsequent communication with the newly connected client.
///			 Current socket remains. Will be used for further accept() calls as they come in.
///			 The client_ev epoll is localy allocated, set up the correct values and then
///			 the client fd and data is added to epoll.
///			 The epoll_event is then destroyed localy.
///			 The fd of the new client is stored inside a member var for later safe checks.
///			 Finally a new Client is allocated calling the Parameter Constructor taking only the fd
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
	this->_fd_to_route[fd_client] = this->_fd_to_route[curr_socket_fd];
	struct epoll_event client_ev;
	client_ev.events = EPOLLIN;
	client_ev.data.fd = fd_client;
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, fd_client, &client_ev);
	Client *newClient = new Client(fd_client);
	this->_clients.insert(std::make_pair(fd_client, newClient));
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
		Client* c = _clients[fd];
		c->appendToReadBuffer(buffer, bytes_read);
		while (request_is_complete(c))
			this->handleCompleteRequest(fd);
	}
}

/// @brief Removes client from epoll, also cleaning the buffer and response vectors
/// @param fd The client fd to be removed.
///	@details 	ALWAYS remove from epoll before closing the FD
///				ALWAYS remove from fd_to_route
void	Server::clientDisconnect( int fd )
{
	epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	this->_fd_to_route.erase(fd);

	// Check before errasing
	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it != this->_clients.end())
	{
		delete it->second;
		this->_clients.erase(it);
	}
}

/// @brief  Extracts and processes a single, complete HTTP request from a client's buffer.
///         Appends the generated HTTP response to the client_response vector and switches the socket
///         to EPOLLOUT so the server knows it is ready to send data.
/// @param fd The client socket file descriptor.
void    Server::handleCompleteRequest( int fd )
{
	Client* c = _clients[fd];
    //+ 1. Isolate the first complete request from the persistent string (handles pipelining)
    std::string single_request = extract_request(c);
    
    //+ 2. Parse the HTTP, find the file/CGI, and build the full string response
    std::string single_response = process_and_build_response(c);
            
    //+ 3. Push the response to the outbound waiting line
    c->appendToWriteBuffer(single_response);
    
    //+ 4. Delete only the parsed request, leaving any leftover bytes for the next cycle
    erase_request_from_buffer(c);
    
    //+ 5. Wake up epoll_wait and tell it we want to write to this client
    if (!c->getWriteBuffer().empty())
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
    Client* c = _clients[fd];
    int sent_bytes = send(fd, c->getWriteBuffer().c_str(), c->getWriteBuffer().length(), 0);

    if (sent_bytes <= 0)
    {
        this->clientDisconnect(fd);
        return ;
    }
    c->eraseWriteBuffer(sent_bytes);
    if (c->getWriteBuffer().empty())
        this->setEpollInOut(fd, EPOLLIN);
}
