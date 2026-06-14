#include "../../includes/WebServ.hpp"

// --- Exceptions ---
Socket::runtimeSocketException::runtimeSocketException(const char* message) : std::runtime_error(message) {}

// --- Set up Epoll ---

/// @brief Add a single Socket to the _socket Vector.
/// @param S The socket to add
void	Server::addSocket( Socket &S ) { this->_sockets.push_back(S); }

/// @brief	Creates the epoll object.
///			Stores the fd into _epoll_fd.
///	@note	Think of it as a list you're going to fill with fds to watch — Close it when you're done.
/// @throw	runtimeServerException() if epoll_create(1) fails.
void	Server::epolInit( void )
{
	this->_epoll_fd = epoll_create(1);
    if (this->_epoll_fd == -1)
		throw runtimeServerException("Error\nEpol init.");
}

/// @brief  Add the sockets to epoll to watch for new connections
/// @throw	runtimeServerException() if epoll_ctl() fails.
void    Server::epollEvent( void )
{
	std::vector<Socket>::iterator it = this->_sockets.begin();

	while (it != this->_sockets.end())
	{
		this->_event.events = EPOLLIN; 
		this->_event.data.fd = it->getFd(); 
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, it->getFd(), &this->_event) == -1)
			throw runtimeServerException("Error\nepoll_ctl()");
		++it;
	}
}

/// @brief Helper to close at the end all Sockets FD's
void	Server::closeSockets( void )
{
	std::vector<Socket>::iterator it = this->_sockets.begin();

	while (it != this->_sockets.end())
	{
		it->closeFd();
		++it;
	}
}

/// @brief Close Epoll FD
void	Server::closeEpoll( void ) { close(this->_epoll_fd); }

// --- Main Loop ---

void	Server::initServer( void )
{
	while (1)
	{
		
	}
}