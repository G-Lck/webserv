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
void	AServer::configAServer( GlobalConfig &config )
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
void	AServer::addSocket( Socket *S )
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

bool	AServer::fdMatch( int curr ) const
{
	return (this->_sockets.find(curr) != this->_sockets.end());
}

bool	AServer::addNewClient( int curr_socket_fd )
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

	Client *newClient = new Client(fd_client);
	this->_clients.insert(std::make_pair(fd_client, newClient));
	try
	{
		this->addClientToMultiplexer(fd_client);
	}
	catch (...)
	{
		this->_clients.erase(fd_client);
		delete newClient;
		this->_fd_to_route.erase(fd_client);
		close(fd_client);
		throw;
	}
	return (true);
}

void	AServer::readRequest( int fd )
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

void	AServer::clientDisconnect( int fd )
{
	this->removeFdFromMultiplexer(fd);
	close(fd);
	this->_fd_to_route.erase(fd);

	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it != this->_clients.end())
	{
		delete it->second;
		this->_clients.erase(it);
	}
}

void	AServer::handleCompleteRequest( int fd )
{
	Client* c = _clients[fd];
	extract_request(c);
	std::string single_response = process_and_build_response(c);

	c->appendToWriteBuffer(single_response);
	erase_request_from_buffer(c);

	if (!c->getWriteBuffer().empty())
		this->watchForWrite(fd);
}

void	AServer::sendResponse( int fd )
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
		this->watchForRead(fd);
}