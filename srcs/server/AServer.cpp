#include "../../includes/AServer.hpp"

// --------------- Orthodoxy ---------------

AServer::AServer() {}

AServer::~AServer()
{
	closeClients();
	closeSockets();
}

// --------------- Exceptions ---------------

AServer::runtimeAServerException::runtimeAServerException(const char* message) : std::runtime_error(message) {} 

// --------------- INITIALIZATION ---------------

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

				std::stringstream ss;
				ss << (*newSocket) << " --> Connection Success";
				writeLog(ss.str(), SERVER_EVENTS); //+ Log adding a socket
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


// --------------- LAUNCHING ---------------


void	AServer::initAServer(void)
{
	this->initMultiplexer();
	this->addSocketsToMultiplexer();
	this->run();
}


// --------------- ADDING A CLIENT ---------------

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

		std::ostringstream oss;
		oss << "accept() failed: on Client creation --> strerror(errno): " << strerror(errno);
		writeLog(oss.str(), SERVER_EVENTS); //+ Log Client error

		return (false);
	}
	// Try to set to non-block
	if (fcntl(fd_client, F_SETFL, O_NONBLOCK) == -1)
	{
		std::ostringstream oss;
		oss << "fcntl() failed: on Client creation --> strerror(errno): " << strerror(errno);
		writeLog(oss.str(), SERVER_EVENTS); //+ Log Client error

		close(fd_client);
		return (false);
	}

	// Create the new client with the fd given by accept() 
	Client *newClient = new Client(fd_client, _fd_to_route[curr_socket_fd]);

	// Set the time for timeout 
	newClient->updateTime();

	// Set Remote address
	newClient->setRemoteAddress(inet_ntoa(addr_client.sin_addr));

	// Insert the client in the map fd:Client 
	this->_clients.insert(std::make_pair(fd_client, newClient));

	// Try to add this fd to epoll
	if(!addClientToMultiplexer(fd_client))
	{
		delete newClient;
		close(fd_client);
		return (false);
	}
	std::stringstream ss;
	ss << (*newClient) << " -> Connetion Success";
	writeLog(ss.str(), SERVER_EVENTS); //+ Log adding a client
	return (true);
}

// ---------------  READING  ---------------

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
	//+	recv(): receives messages from a socket
	int bytes_read = recv(fd, buffer, sizeof(buffer), 0);

	//+ man recv(): When a stream socket peer has performed an orderly shutdown, the
    //+ return value will be 0 (the traditional "end-of-file" return)
	if (bytes_read == 0)
	{
		this->clientDisconnect(fd);
	}
	else if (bytes_read < 0)
	{
		//+ man recv(): EAGAIN or EWOULDBLOCK: The socket is marked nonblocking and the receive operation
		//+ would block, or a receive timeout had been set and the
		//+ timeout expired before data was received.
		//+ EINTR  The receive was interrupted by delivery of a signal before any data was available
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		{
			std::stringstream ss;
			ss << "recv() error -> " << getRecvErrorStr(errno);
			writeLog(ss.str(), ERROR_WARNING);
			return ;
		}
		//+ In any other case we log error
		std::stringstream ss;
		ss << "recv() error -> " << getRecvErrorStr(errno);
		writeLog(ss.str(), ERROR_INFO);
		this->clientDisconnect(fd);
	}
	else
	{
		//+ Tries to match the fd, with the fd on the Clients map
		std::map<int, Client*>::iterator it = _clients.find(fd);
		// If not found we log error and continue
		if (it == _clients.end())
		{
			writeLog("readRequest: unknown fd", ERROR_INFO);
			return ;
		}
		//+ Otherwise we append to buffer until the request is finished
		Client* c = it->second;
		c->appendToReadBuffer(buffer, bytes_read);

		try
		{
			while (c->parseBufferedRequest() == true)
			{
				std::stringstream ss;
				ss << (*c) << " -> Complete request recieved";
				writeLog(ss.str(), SERVER_EVENTS); //+ Log request
				
				std::stringstream sss;
				sss << c->getRequest();
				writeLog(sss.str(), ACCESS);

				//+ This case is CGI init, no reponse created
				if (!this->handleCompleteRequest(fd))
					break ;
			}
		}
		catch (const HttpException& e)
		{
			std::stringstream ss;
			ss << e.getCode() << ": " << e.what();
			writeLog(ss.str(), STATUS_CODE);
			//~ HERE WE MISS A FUNCTION TO EIRTHER GET THE PAGE FROM LOCATION
			//~ OR CALL:
			finishRequestCycle(c); //+ Cleanup http request
			this->prepareToSend(fd, c, e.getResponseStr());
		}
	}
}

/// @brief	As the function name describes, We handle a request was all succesfully read
///			and stored in the body of the Client.
///			We build the HttpRequest and then send it to the handler -> if the request
///			Asks for a cgi handler, we init the handler, create the child process and
///			return to the main loop.
///			Else we handle with executeStatic() and we create the response string
///			2 Cases if throw we create the error by checking if error_pages exists or we
///			return a defualt error response sting. otherwise we create the ok response string.
/// @param fd Client
/// @return true → response built (static or error path) | false → CGI just started, nothing to send yet
bool	AServer::handleCompleteRequest(int fd)
{
	std::string	single_response;
	
	//+ Look for this Client in the map
	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
	{
		writeLog("Request: unknown fd at handleCompleRequest, possible race", ERROR_INFO);
		return false;
	}
	Client* c = it->second;
	
	//+ Build the HTTP response for this request.
	t_virtualServer::iterator route_it = this->_virtualServers.find(c->getHostPort());
	if (route_it == this->_virtualServers.end())
	{
		HttpException e(500, "500: Internal Server Error: handleCompleRequest()");
		writeLog("500: Internal Server Error: handleCompleRequest()", STATUS_CODE);
		finishRequestCycle(c); //+ Cleanup http request
		this->prepareToSend(fd, c, e.getResponseStr());
		return true;
	}

	//+ Parse the request
	Handler		handler(c);
	try
	{
		handler.initAndParseHandler(route_it->second); //* this may throw
	}
	catch(const HttpException& e)
	{
		std::stringstream ss;
		ss << e.getCode() << ": " << e.what();
		writeLog(ss.str(), STATUS_CODE);
		//~ HERE WE MISS A FUNCTION TO EIRTHER GET THE PAGE FROM LOCATION
		//~ OR CALL:
		single_response = e.getResponseStr();
		finishRequestCycle(c); //+ Cleanup http request
		this->prepareToSend(fd, c, single_response);
		return true;
	}

	//+ Select type of Handler according to the request
	try
	{
		//+ Static Handler: always a response ready even if it fails. So we return after
			//handler.executeStatic(); //* this may throw (only if you dont find the error page inside location)
			//single_response = handler->client->_response.buildResponseStr();
			single_response = process_and_build_response(c);
			finishRequestCycle(c); //+ Cleanup http request
			this->prepareToSend(fd, c, single_response);
			return true;
		//+ Cgi Handler: We init the allocated CgiHandler, if it throws, the catch deletes it and send the response
		//+ On success, we create the handler, the child process and go back to the main loop to check for write to the cgi
	

	}
	catch(const HttpException& e)
	{
		std::stringstream ss;
		ss << e.getCode() << ": " << e.what();
		writeLog(ss.str(), STATUS_CODE);
		//~ HERE WE MISS A FUNCTION TO EIRTHER GET THE PAGE FROM LOCATION
		//~ OR CALL:
		single_response = e.getResponseStr();
		finishRequestCycle(c); //+ Cleanup http request
		this->prepareToSend(fd, c, single_response);
		return true;
	}
	return false;
}

/// @brief Cleaning up the request class, Set keep alive according to the request
void AServer::finishRequestCycle(Client* c)
{
    c->eraseProcessedRequest();
    c->wantsKeepAlive();
    c->resetCurrentRequest();
}

/// @brief	Lasts steps for creating a response string, this funtion will work arround the response created
///			string, adding it to the queue and changing to EPOLLOUT
///			in case of needed, so in the next iteration we can send the response.
/// @param fd The client file descriptor for that request processed
/// @param single_response	The string with the full response. It can be a succes obtained from a Handler
///							or an exception default-build string obtained from HttpException::buildDefaultResponse() 

void AServer::prepareToSend(int fd, Client* c, std::string single_response)
{
    c->addResponseQueue(single_response);

	//+ Check if needs to be queued before sending
	//+ leaves the new response queued. sendResponse will pick it up later when the current one finishes.
    if (c->isWriteBufferEmpty() && !c->isResponseQueueEmpty())
    {
        c->appendToWriteBuffer(c->frontResponse());
        c->popFrontResponse();
        this->watchForWrite(fd);
    }
}


// ---------------  RESPONDING  ---------------


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
			std::stringstream ss;
			ss << "send() error -> " << getRecvErrorStr(errno);
			writeLog(ss.str(), ERROR_WARNING);
			return ;
		}
		std::stringstream ss;
		ss << "send() error -> " << getRecvErrorStr(errno);
		writeLog(ss.str(), ERROR_INFO);
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


// ---------------   CLEANUP   ---------------


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
		std::stringstream ss;
		ss << *(it->second) << " -> Client Disconnect";
		writeLog(ss.str(), SERVER_EVENTS);

		delete it->second;
		this->_clients.erase(it);
	}
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


// --------------- CLIENT MONITORING ---------------


void	AServer::refreshClientTime( int fd )
{
	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
		return ;
	it->second->updateTime();
}

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
			oss << *(it->second) << " --> Client Timeout";
			writeLog(oss.str(), SERVER_EVENTS); //+ Log Client timeout

			this->clientDisconnect(it->first);
		}
		it = next;
	}
}

