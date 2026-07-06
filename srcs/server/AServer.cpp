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
void	AServer::configAServer(GlobalConfig &config)
{
	t_port_host	 used_ports;
	for (int i = 0; i < (int)config.serverCount(); i++)
	{
		const ServerConfig	&ServConf = config.getServers(i);
		const t_port_host	ports = ServConf.getAllListen();

		for (int j = 0; j < (int)ports.size(); j++)
		{
			t_port_host::iterator it = std::find(used_ports.begin(), used_ports.end(), ports[j]);
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

/// @brief Add a single Socket to the server list.
void	AServer::addSocket(Socket *S)
{
	this->_sockets.insert(std::make_pair(S->getFd(), S));
}

/// @brief Close and delete all sockets.
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

/// @brief Close and delete all remaining clients.
void	AServer::closeClients(void)
{
	std::map<int, Client*>::iterator it = this->_clients.begin();
	while (it != this->_clients.end())
	{
		close(it->first);
		delete it->second;
		++it;
	}
	this->_clients.clear();
}

// --------------- Launching ---------------

void	AServer::initAServer(void)
{
	this->initMultiplexer();
	this->addSocketsToMultiplexer();
	this->run();
}

// --------------- Handle Cases ---------------

bool	AServer::fdMatch(int curr) const
{
	return (this->_sockets.find(curr) != this->_sockets.end());
}

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
	if (fcntl(fd_client, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cout << "fcntl() failed: " << strerror(errno) << std::endl; //~asdasdasd
		close(fd_client);
		return (false);
	}
	this->_fd_to_route[fd_client] = this->_fd_to_route[curr_socket_fd];

	Client *newClient = new Client(fd_client);
	this->_clients.insert(std::make_pair(fd_client, newClient));
	if(!addClientToMultiplexer(fd_client))
	{
		this->_clients.erase(fd_client);
		delete newClient;
		this->_fd_to_route.erase(fd_client);
		close(fd_client);
		return (false);
	}
	return (true);
}

/// @brief	This function willccall recv, and either handle the errors
///			or on success, call appendToReadBuffer(), to add the read request from the buffer
///			to the client matching with the fd passed as parameter.
///			Finally it will call handleCompleteRequest() to handle the Requests.
/// @param fd The client to match.
void    AServer::readRequest(int fd)
{
	char buffer[4096];
	int bytes_read = recv(fd, buffer, sizeof(buffer), 0);

	if (bytes_read == 0)
	{
		this->clientDisconnect(fd);
	}
	else if (bytes_read < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			return ;
		logError(getRecvErrorStr(errno), 1);
		this->clientDisconnect(fd);
	}
	else
	{
		std::map<int, Client*>::iterator it = _clients.find(fd);
		if (it == _clients.end())
		{
			logError("readRequest: unknown fd", 1);
			return ;
		}
		Client* c = it->second;
		c->appendToReadBuffer(buffer, bytes_read);
		while (c->parseBufferedRequest() == true)
			this->handleCompleteRequest(fd);
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
		logError(err.append(strerror(errno)), 1);
	}
	this->_fd_to_route.erase(fd);

	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it != this->_clients.end())
	{
		delete it->second;
		this->_clients.erase(it);
	}
}

void	AServer::handleCompleteRequest(int fd)
{
	Client* c = _clients[fd];
	extract_request(c);
	std::string single_response = process_and_build_response(c);

	c->addResponseQueue(single_response);
	erase_request_from_buffer(c);
	c->resetCurrentRequest();

	if (c->getWriteBuffer().empty() && !c->isResponseQueueEmpty())
	{
		c->appendToWriteBuffer(c->frontResponse());
		c->popFrontResponse();
		this->watchForWrite(fd);
	}
}

void    AServer::sendResponse(int fd)
{
	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
	{
		logError("sendResponse: unknown fd", 1);
		return ;
	}
	Client* c = it->second;

	int sent_bytes = send(fd, c->getWriteBuffer().c_str(), c->getWriteBuffer().length(), MSG_NOSIGNAL);
	if (sent_bytes == 0)
	{
		this->clientDisconnect(fd);
		return ;
	}
	if (sent_bytes < 0)
	{
		logError(getSendErrorStr(errno), 1);
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			return ;
		this->clientDisconnect(fd);
		return ;
	}

	c->eraseWriteBuffer(sent_bytes);
	if (!c->getWriteBuffer().empty())
	{
		this->watchForWrite(fd);
		return;
	}
	if (!c->isResponseQueueEmpty())
	{
		c->appendToWriteBuffer(c->frontResponse());
		c->popFrontResponse();
		this->watchForWrite(fd);
	}
	else
	{
		this->watchForRead(fd);
	}
}
