#include "../../includes/AServer.hpp"

// --------------- Orthodoxy ---------------

AServer::AServer() {}

AServer::~AServer()
{
	closeClients();
	closeSockets();
}

// --------------- Exceptions ---------------

AServer::runtimeAServerException::runtimeAServerException(const char* message)
	: std::runtime_error(message) {}

// --------------- Set up Config ---------------

/// @brief Initializes physical sockets and sets up the virtual host routing maps.
///			Taking data from th parsed Globalconfig, we intialize a loop to create
///			the sockets:
///				Double loop: one socket per "server"keyword per combo Host::Port found in the config file.
///			Finally, we add to the virtual servers, a new t_virualServer type:
///				This stores this data: Host:Port combo and a vector to all ServerConfig
void	AServer::configAServer(GlobalConfig &config)
{
	t_port_host	 used_ports;
	for (int i = 0; i < (int)config.serverCount(); i++)
	{
		const ServerConfig	&ServConf = config.getServers(i);
		const t_port_host	ports = ServConf.getAllListen();

		for (int j = 0; j < (int)ports.size(); j++)
		{
			// Try to find the current port to be added in the list of already used ports
			t_port_host::iterator it = std::find(used_ports.begin(), used_ports.end(), ports[j]);
			// If not found, add a new socket
			if (it == used_ports.end())
			{
				Socket *newSocket = new Socket;
				newSocket->makeSocket(ports[j].first, ports[j].second);
				this->_fd_to_route[newSocket->getFd()] = ports[j];
				used_ports.push_back(ports[j]);
				this->addSocket(newSocket);
				std::cout << ports[j].first << "::" << ports[j].second << std::endl;
			}
			this->_virtualServers[ports[j]].push_back(ServConf);
		}
	}
}

/// @brief	Add a single Socket to the server list.
///			We store in the map of (fd, Socket)
void	AServer::addSocket(Socket *S)
{
	this->_sockets.insert(std::make_pair(S->getFd(), S));
}

/// @brief	Close and delete all sockets.
///			Iterate in the map of Sockets, closing the fd and freeing the memory
void	AServer::closeSockets(void)
{
	std::map<int, Socket*>::iterator it = this->_sockets.begin();
	while (it != this->_sockets.end())
	{
		it->second->closeFd();
		delete it->second;
		++it;
	}
	this->_sockets.clear();
}

/// @brief	Close and delete all remaining clients.
///			Iterate in the map of Cliets, closing the fd and freeing the memory
void	AServer::closeClients(void)
{
	std::map<int, Client*>::iterator it = this->_clients.begin();
	while (it != this->_clients.end())
	{
		it->second->closeFd();
		delete it->second;
		++it;
	}
	this->_clients.clear();
}
// --------------- Monitoring ---------------

bool	AServer::clientTimeout( int fd )
{
	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
		return (false);
	time_t	last_activity = it->second->getLastActivity();
	time_t	now;
	time(&now);

	if (difftime(now, last_activity) > MAX_WAIT_TIME)
		return (true);
	return(false);
}

/// @brief Funtion to monitor client timeouts. If any timneout it will get disconected
void	AServer::monitorClients()
{
	std::map<int, Client*>::iterator it = this->_clients.begin();
	while (it != this->_clients.end())
	{
		std::map<int, Client*>::iterator next = it;
		++next;
		if (clientTimeout(it->first))
		{
			std::ostringstream oss;
			oss << "Client timeout, disconnecting fd " << it->first << std::endl;
			writeLog(oss.str(), SERVER_EVENTS);
			this->clientDisconnect(it->first);
		}
		it = next;
	}
}

// --------------- Launching ---------------

void	AServer::initAServer(void)
{
	this->initMultiplexer();
	this->addSocketsToMultiplexer();
	this->run();
}

// --------------- Handle Cases ---------------

/// @brief 	Simple helper function retuning true if the curr
///			matches with an fd of the list of Sockets
/// @param curr The fd to be found
bool	AServer::fdMatch(int curr) const
{
	return (this->_sockets.find(curr) != this->_sockets.end());
}

/// @brief	This funtion will add a New client to epoll and to our _clients map
///			It is executed when during the main loop, the fd inside the _active_events
///			genetared by epoll_wait() matches a Socket id.
///			The function will call accept() and with the fd given by this function
///			Will create the new client and pair it as well with the Socket (host:port)
/// @param curr_socket_fd The fd of the Socket to pair the new client with
bool	AServer::addNewClient(int curr_socket_fd)
{
	struct sockaddr_in  addr_client;
	socklen_t           addr_size = sizeof(addr_client);

	int fd_client = accept(curr_socket_fd, (struct sockaddr *)&addr_client, &addr_size);
	if (fd_client == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (false);
		std::cout << "accept() failed: " << strerror(errno) << std::endl; //~asdasdasd
		return (false);
	}
	// Try to set to non-block
	if (fcntl(fd_client, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cout << "fcntl() failed: " << strerror(errno) << std::endl; //~asdasdasd
		close(fd_client);
		return (false);
	}

	// Create the new client with the fd given by accept()
	Client *newClient = new Client(fd_client, _fd_to_route[curr_socket_fd]);

	// Set the time for timeout
	newClient->updateTime();

	// Insert the client in the map fd:Client
	this->_clients.insert(std::make_pair(fd_client, newClient));

	// Try to add this fd to epoll
	if(!addClientToMultiplexer(fd_client))
	{
		delete newClient;
		close(fd_client);
		return (false);
	}
	return (true);
}

/// @brief	This function will call recv, and either handle the errors
///			or on success:
///			Case 1: The request is not yet complete:
///				 call appendToReadBuffer(), to add the read request from the buffer to the client body.
///			Case 2: The request is compelted
///				call handleCompleteRequest() to handle the request and switch to EPOLLOUT
/// @param fd The client to match.
void    AServer::readRequest(int fd)
{
	char buffer[4096];
	//	recv(): receives messages from a socket
	int bytes_read = recv(fd, buffer, sizeof(buffer), 0);

	// man recv(): When a stream socket peer has performed an orderly shutdown, the
    // return value will be 0 (the traditional "end-of-file" return)
	if (bytes_read == 0)
	{
		this->clientDisconnect(fd);
	}
	else if (bytes_read < 0)
	{
		// man recv(): EAGAIN or EWOULDBLOCK: The socket is marked nonblocking and the receive operation
		// would block, or a receive timeout had been set and the
		// timeout expired before data was received.
		// EINTR  The receive was interrupted by delivery of a signal before any data was available
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		{
			writeLog(getRecvErrorStr(errno), ERROR_WARNING); //~ HERE I THINK WE SHOULD LOG IN ANOTHER FILE, LIKE A WARNING AND NOT AN ERROR, THE CLIENT WONT BE DISCONECTED
			return ;
		}
		// In any other case we log error
		writeLog(getRecvErrorStr(errno), ERROR_INFO);
		this->clientDisconnect(fd);
	}
	else
	{
		// Tries to match the fd, with the fd on the Clients map
		std::map<int, Client*>::iterator it = _clients.find(fd);
		// If not found we log error and continue
		if (it == _clients.end())
		{
			writeLog("readRequest: unknown fd", ERROR_INFO);
			return ;
		}
		// Otherwise we append to buffer until the request is finished
		Client* c = it->second;
		c->appendToReadBuffer(buffer, bytes_read);
		while (c->parseBufferedRequest() == true)
		{
			this->handleCompleteRequest(fd);
			if (!c->getKeepAlive())
			{
				this->clientDisconnect(fd);
				break;
			}
		}
	}
}

/// @brief	We check on close for -1 in rare case where send() error code is EPIPE. Flag is set in send to MSG_NOSIGNAL to mute
///			the signal sent by the kernell, and instead of killing the program we handle the errors.
///			EPIPE will be sent in the case where the client disconnects mid-read (possible double close, we log error just in case). 
void	AServer::clientDisconnect(int fd)
{
	this->removeFdFromMultiplexer(fd);
	if (close(fd) == -1)
	{
		std::string err = "Client already disconnected: ";
		writeLog(err.append(strerror(errno)), ERROR_INFO);
	}

	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it != this->_clients.end())
	{
		delete it->second;
		this->_clients.erase(it);
	}
}

/// @brief	As the function name describes, We handle a request was all succesfully read
///			and stored in the body of the Client.
/// @param fd Client
void	AServer::handleCompleteRequest(int fd)
{
	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
	{
		writeLog("Request: unknown fd", ERROR_INFO);
		return ;
	}
	Client* c = it->second;
	// Build the HTTP response for this request.

	std::string single_response = process_and_build_response(c);

	// Clean up: erase the processed request from the read buffer, resets state for the next request.
	c->addResponseQueue(single_response);
	c->eraseProcessedRequest();
	c->wantsKeepAlive(); // Set the _keep_alive member variable before reseting the request
	c->resetCurrentRequest();

	// Check if needs to be queued before sending
	// leaves the new response queued. sendResponse will pick it up later when the current one finishes.
	if (c->isWriteBufferEmpty() && !c->isResponseQueueEmpty())
	{
		c->appendToWriteBuffer(c->frontResponse());
		c->popFrontResponse();
		this->watchForWrite(fd);
	}
}

/// @brief	Called when EPOLLOUT fires (socket ready to write):
///			
/// @param fd Client
void    AServer::sendResponse(int fd)
{
	// Search for that client
	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
	{
		writeLog("sendResponse: unknown fd", ERROR_INFO);
		return ;
	}
	Client* c = it->second;

	// Send the response!!
	int sent_bytes = send(fd, c->getWriteData(), c->getWriteRemaining(), MSG_NOSIGNAL);
	if (sent_bytes < 0)
	{
		// man recv(): EAGAIN or EWOULDBLOCK: The socket is marked nonblocking and the receive operation
		// would block, or a receive timeout had been set and the
		// timeout expired before data was received.
		// EINTR  The receive was interrupted by delivery of a signal before any data was available
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		{
			writeLog(getSendErrorStr(errno), ERROR_WARNING); //~ HERE I THINK WE SHOULD LOG IN ANOTHER FILE, LIKE A WARNING AND NOT AN ERROR, THE CLIENT WONT BE DISCONECTED
			return ;
		}
		writeLog(getSendErrorStr(errno), ERROR_INFO);
		this->clientDisconnect(fd);
		return ;
	}

	// On success: erase what was sent from the buffer. 
	c->eraseWriteBuffer(sent_bytes);

	// If bytes remain, keep watching for write.
	if (!c->isWriteBufferEmpty())
	{
		this->watchForWrite(fd);
		return;
	}
	// If empty:
	// Case 1: Load the next queued response and keep watching write
	if (!c->isResponseQueueEmpty())
	{
		c->appendToWriteBuffer(c->frontResponse());
		c->popFrontResponse();
		this->watchForWrite(fd);
	}
	// Case 2: Switch back to watching read (waiting for the client's next request).
	else
	{
		if (c->getKeepAlive())
			this->watchForRead(fd);
		else
			this->clientDisconnect(fd);
	}
}
