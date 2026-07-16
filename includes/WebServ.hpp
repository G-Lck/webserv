#ifndef WEBSERV_HPP
# define WEBSERV_HPP

//* --- DEFINES ---

#define RED     "\033[31m"
#define BLACK   "\033[0m"
#define GREEN   "\033[32m"
#define PURPLE  "\033[35m"
#define YELLOW  "\033[33m"

#define MAX_BODY 104857600 // 10MB

#define DEFAULT_HOST "0.0.0.0"
#define DEFAULT_PORT "8080"
#define MAX_WAIT_TIME 5			// Time in seconds, how long we wait until closing a Client
#define SERVER_REFRESH_TIME 500		// Time in miliseconds, basically how often epoll_wait() is called in the main loop

//* --- LOG DEFINES ---

enum LogFiles
{
	INFO,			// Has Global config after parsing, maybe we can also add failed parsings
	ACCESS,			// Has nothing so far
	SERVER_EVENTS,	// Has client and socket connection, client disconection, cleint timout, logs a full request
	ERROR_INFO,		// Has severe errors (weird stuff)
	ERROR_WARNING,	// Has minor errors	(more like functions that we need to use that have unexpected cases)
	STATUS_CODE,	// Self explained
};

// --- C++ Standard Core ---
#include <iostream>     // std::cout, std::cerr
#include <string>       // std::string
#include <vector>       // std::vector (perfect for managing your pollfd roster)
#include <cstring>      // std::memset(), std::strlen()
#include <utility>		// std::pair
#include <map>			// std::map
#include <deque>		// std::deque
#include <stdexcept>	// exceptions
#include <algorithm>
#include <cerrno>  		// Required to read the 'errno' variable
#include <cstring> 		// Required for strerror()
#include <fstream>
#include <sstream>		// to us istringstream easier to split on spaces
#include <cstddef>		// core fundamental types and macros


// --- POSIX Sockets & Network ---
#include <sys/socket.h> // socket(), bind(), listen(), accept(), send(), recv(), setsockopt()
#include <netdb.h>      // getaddrinfo(), freeaddrinfo(), struct addrinfo
#include <netinet/in.h> // struct sockaddr_in, htons(), htonl(), ntohs(), ntohl()

// --- POSIX Multiplexing & System Calls ---
#include <poll.h>       // poll(), struct pollfd, POLLIN, POLLOUT
#include <fcntl.h>      // fcntl(), F_SETFL, O_NONBLOCK
#include <unistd.h>     // close(), read(), write(), fork(), execve(), pipe(), dup2()
#ifdef __linux__
# include <sys/epoll.h> 	// epoll()
#endif

// --- POSIX File System & CGI ---
#include <sys/stat.h>	// stat() (to check if an HTML file exists before serving)
#include <unistd.h>
#include <dirent.h>		// opendir(), readdir(), closedir() (for autoindex)

// --- POSIX signals ---
#include <signal.h>

// --- POSIX time ---
#include <ctime>


class GlobalConfig;

bool	validBrackets(const std::string &text);
bool	isValidErrorPagePath(const std::string& path);
bool	validListen(const std::string &token);
bool	validIP(const std::string &host);
bool	validMethod( std::string method );
void	printConfig(GlobalConfig &config, int flag);

#endif