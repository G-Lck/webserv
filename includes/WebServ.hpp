#ifndef WEBSERV_HPP
# define WEBSERV_HPP

//* --- DEFINES ---

#define RED     "\033[31m"
#define BLACK   "\033[0m"
#define GREEN   "\033[32m"
#define PURPLE  "\033[35m"
#define YELLOW  "\033[33m"

// --- C++ Standard Core ---
#include <iostream>     // std::cout, std::cerr
#include <string>       // std::string
#include <vector>       // std::vector (perfect for managing your pollfd roster)
#include <cstring>      // std::memset(), std::strlen()
#include <utility>		// std::pair
#include <map>			// std::map
#include <stdexcept>	// exceptions
#include <algorithm>
#include <cerrno>  		// Required to read the 'errno' variable
#include <cstring> 		// Required for strerror()

// --- POSIX Sockets & Network ---
#include <sys/socket.h> // socket(), bind(), listen(), accept(), send(), recv(), setsockopt()
#include <netdb.h>      // getaddrinfo(), freeaddrinfo(), struct addrinfo
#include <netinet/in.h> // struct sockaddr_in, htons(), htonl(), ntohs(), ntohl()

// --- POSIX Multiplexing & System Calls ---
#include <poll.h>       // poll(), struct pollfd, POLLIN, POLLOUT
#include <fcntl.h>      // fcntl(), F_SETFL, O_NONBLOCK
#include <unistd.h>     // close(), read(), write(), fork(), execve(), pipe(), dup2()
#include <sys/epoll.h> 	// epoll()

// --- POSIX File System & CGI ---
#include <sys/stat.h>   // stat() (to check if an HTML file exists before serving)
#include <dirent.h>     // opendir(), readdir(), closedir() (for autoindex)

//* ---------- HPP Includes -----------

#include "Socket.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include "HttpRequest.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include "GlobalConfig.hpp"
#include "Client.hpp"

#endif