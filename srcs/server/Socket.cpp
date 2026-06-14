#include "../../includes/WebServ.hpp"

//* THIS IS JUST A HELPER FOR NOW, WE NEED TO SEE WHAT WE PASS TO THIS.
void Socket::setConfig( void )
{
	this->_config.listen = "8002";
	this->_config.server_name = "localhost";
	this->_config.host = "127.0.0.1";
	this->_config.root = "./www/";
	this->_config.index = "index.html";
}

// --- Orthodox ---

Socket::Socket() {}
Socket::~Socket() {}
Socket::Socket( Socket const &other ) : _status(other._status), _fd(other._fd), _hints(other._hints), _socket(other._socket) {}
Socket &Socket::operator=( Socket const &other )
{
	if (this != &other)
	{
		_status = other._status;
		_fd = other._fd;
		_hints = other._hints;
		_socket = other._socket;
	}
	return ( *this );
}

// --- Exceptions ---
Socket::runtimeSocketException::runtimeSocketException(const char* message) : std::runtime_error(message) {}

// --- Socket creation (in execution order) ---

/// @brief	This function is run as a pre-setup for calling the function socket()
///			Here we pre determinate some values such as:
///			AF_UNSPEC (Unsespected IPv4 or IPv6) | 
///			SOCK_STREAM (Specifies that is a TCP stream socket) | 
///			AI_PASSIVE (Sets the ip of this pc for us) | 
/// @throw	Throws runtimeSocketException() if getaddrinfo() fails
void Socket::socketGetAddrInfo( void )
{
	memset(&(this->_hints), 0, sizeof this->_hints);
	this->_hints.ai_family = AF_UNSPEC;
	this->_hints.ai_socktype = SOCK_STREAM;
	this->_hints.ai_flags = AI_PASSIVE;

	this->_status = getaddrinfo(NULL, this->_config.listen.c_str(), &(this->_hints), &(this->_socket));
	if (this->_status != 0)
		throw runtimeSocketException("Error:\ngetaddrinfo() Fail");
}

/// @brief	Calls the function socket() to set the _fd
/// @throw	Throws runtimeSocketException() if socket() fails
void	Socket::socketCall( void ) 
{
	this->_fd = socket(this->_socket->ai_family, this->_socket->ai_socktype, this->_socket->ai_protocol);
	if (this->_fd == -1)
		throw runtimeSocketException("Error:\nsocket() Fail");
}

/// @brief	Calls setsockopt, to avoid "Address already in use" error.
//			In case of server stopping, this lets bind assing the same port from before.
//			This is because if the server stops, the OS keeps this port for 60 seconds after. With this we avoid that
/// @throw	Throws runtimeSocketException() if setsockopt() fails
void	Socket::socketOpt( void ) 
{
    int opt = 1;
    if (setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) 
		throw runtimeSocketException("Error:\nsetsockopt() Fail");
}

/// @brief	Calls bind() function for this socket.
///			bind() assigns the IP and Port < set by Socket::getAddrInfo() > to the FD of this socket
/// @throw	Throws runtimeSocketException() if bind() fails
void	Socket::socketBind( void ) 
{
	if (bind(this->_fd, this->_socket->ai_addr, this->_socket->ai_addrlen) == -1)
		throw runtimeSocketException("Error:\nbind() Fail");
}

/// @brief	Calls freeaddrinfo() function for this socket.
///			Frees the linked list memory created by Socket::getAddrInfo()
/// @note	Free after using bind(), not needed after.
void	Socket::socketFreeAddrInfo( void ) { freeaddrinfo(this->_socket); }

/// @brief	Calls listen() for this socket.
//			listen() tells the OS to start accepting incoming connections. 
/// @note 	Constant BACKLOG - Reffers to how many incoming connections can wait in line at once.
void	Socket::socketListen( void ) 
{
    if (listen(this->_fd, BACKLOG) == -1)
		throw runtimeSocketException("Error:\nlisten() Fail");
}

/// @brief	calls fcntl() function for this socket
///			fcntl() is called with this flags:
///			F_SETFL => Use to set flags | 
///			O_NONBLOCK => Apply non block to main socket.
///	@note	It forces functions like recv() and accept() to return an error immediately 
///			if there is no data, rather than freezing the program.
void	Socket::socketSetNonBlock( void ) { fcntl(this->_fd, F_SETFL, O_NONBLOCK); }

/// @brief	Calls all the member functions for creating a socket in order.
/// @param backlog Reffers to how many incoming connections can wait in line at once.
void	Socket::makeSocket( void )
{
	this->socketGetAddrInfo();
	this->socketCall();
	this->socketOpt();
	this->socketBind();
	this->socketFreeAddrInfo();
	this->socketListen();
	this->socketSetNonBlock();
}

/// @brief	Close the file descriptor attached to this Socket.
void	Socket::closeFd( void ) { close(this->_fd); }

int		Socket::getFd( void ) { return (this->_fd); }

