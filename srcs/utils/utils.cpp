#include "../../includes/WebServ.hpp"
#include "../../includes/GlobalConfig.hpp"
#include "../../includes/ServerConfig.hpp"
#include "../../includes/LocationConfig.hpp"

bool validBrackets(const std::string &text)
{
	int brackets = 0;
	for (size_t i = 0; i < text.size(); i++)
	{
		if (text[i] == '{') brackets++;
		if (text[i] == '}') brackets--;
		if (brackets < 0)
			return false;
	}
	return brackets == 0;
}

/// @brief Simple boolean function to validate the error page path
/// @param path The string to validate
bool isValidErrorPagePath(const std::string& path)
{
    if (path.empty())
        return false;
    //+ 1. Must start with '/' (absolute path relative to server root)
    if (path[0] != '/')
        return false;
    //+ 2. Must not end with '/' (it must be a file, not a directory)
    if (path[path.length() - 1] == '/')
        return false;
    //+ 3. Must contain a file extension (e.g., .html, .htm)
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == path.length() - 1)
        return false;
    return true;
}

/// @brief Function to validate the format of the IP
/// @param host the string to validate
bool validIP(const std::string &host)
{
	int  dots = 0;
	long num = 0; 
	bool hasDigit = false;

	for (size_t i = 0; i < host.size(); i++)
	{
		if (host[i] == '.')
		{
			if (!hasDigit || num > 255)
				return false;
			dots++;
			num = 0;
			hasDigit = false;
		}
		else if (isdigit(host[i]))
		{
			num = num * 10 + (host[i] - '0');
			if (num > 255) 
				return false; 
			hasDigit = true;
		}
		else
			return false;
	}
	return dots == 3 && hasDigit && num <= 255;
}

/// @brief	Checks if the listen argument is valid for ServerConfig  :
///			8080 → host defaults to 0.0.0.0   ||
///			127.0.0.1:8080   |||
///			localhost:8080
/// @param token the token to validate
bool validListen(const std::string &token)
{
	size_t colon = token.find(':');

	if (colon == std::string::npos)
	{
		if (validIP(token)) 
			return true; 
			
		for (size_t i = 0; i < token.size(); i++)
			if (!isdigit(token[i]))
				return false;
		
		if (token.empty()) return false;
		long port = std::atol(token.c_str());
		return (port >= 1 && port <= 65535);
	}

	std::string host = token.substr(0, colon);
	std::string portStr = token.substr(colon + 1);

	if (host.empty() || portStr.empty())
		return false;
		
	if (portStr.find_first_not_of("0123456789") != std::string::npos)
		return false;
			
	long port = std::atol(portStr.c_str());
	if (port < 1 || port > 65535)
		return false;

	if (validIP(host)) 
		return true;
		
	for (size_t i = 0; i < host.size(); i++)
		if (!isalnum(host[i]) && host[i] != '-' && host[i] != '.')
			return false;

	return true;
}

bool	validMethod( std::string method )
{
	if ( method.compare("GET") == 0 ||
		method.compare("POST") == 0 ||
		method.compare("DELETE") == 0 )
		return (true);
	return (false);
}

/// @brief Prints the whole config
/// @param flag To use 1 when expected to pass, 0 when expected to fail
void printConfig(GlobalConfig &config, int flag)
{
	std::cout << YELLOW << "=== Global Config ===" << BLACK << std::endl;
	std::cout << "  root: " << config.getRoot() << std::endl;
	std::cout << "  autoindex: " << (config.getAutoindex() ? "on" : "off") << std::endl;
	std::cout << "  max_body: " << config.getClientMaxBodySize() << std::endl;

	const std::vector<std::string> &idx = config.getIndex();
	std::cout << "  index:";
	for (size_t i = 0; i < idx.size(); i++)
		std::cout << " " << idx[i];
	std::cout << std::endl;

	const std::map<int, std::string> &ep = config.getErrorPages();
	for (std::map<int, std::string>::const_iterator it = ep.begin(); it != ep.end(); ++it)
		std::cout << "  error_page: " << it->first << " -> " << it->second << std::endl;

	for (size_t s = 0; s < config.serverCount(); s++)
	{
		const ServerConfig &serv = config.getServers(s);
		std::cout << PURPLE << "\n  [server " << s << "]" << BLACK << std::endl;
		std::cout << "    root: " << serv.getRoot() << std::endl;
		std::cout << "    autoindex: " << (serv.getAutoindex() ? "on" : "off") << std::endl;
		std::cout << "    max_body: " << serv.getClientMaxBodySize() << std::endl;
		std::cout << "    cgi: " << serv.getCgiHandler().first << " " << serv.getCgiHandler().second << std::endl;

		const std::vector<std::pair<std::string, std::string> > &listen = serv.getAllListen();
		for (size_t i = 0; i < listen.size(); i++)
			std::cout << "    listen: " << listen[i].first << ":" << listen[i].second << std::endl;

		const std::vector<std::string> &sn = serv.getServerName();
		std::cout << "    server_name:";
		for (size_t i = 0; i < sn.size(); i++)
			std::cout << " " << sn[i];
		std::cout << std::endl;

		const std::vector<std::string> &sidx = serv.getIndex();
		std::cout << "    index:";
		for (size_t i = 0; i < sidx.size(); i++)
			std::cout << " " << sidx[i];
		std::cout << std::endl;

		const std::map<int, std::string> &sep = serv.getErrorPage();
		for (std::map<int, std::string>::const_iterator it = sep.begin(); it != sep.end(); ++it)
			std::cout << "    error_page: " << it->first << " -> " << it->second << std::endl;

		for (size_t l = 0; l < serv.getLocations().size(); l++)
		{
			const LocationConfig &loc = serv.getLocations()[l];
			std::cout << GREEN << "      [location " << loc.getPath() << "]" << BLACK << std::endl;
			std::cout << "        root: " << loc.getRoot() << std::endl;
			std::cout << "        autoindex: " << (loc.getAutoindex() ? "on" : "off") << std::endl;
			std::cout << "        max_body: " << loc.getClientMaxBodySize() << std::endl;
			std::cout << "        cgi: " << serv.getCgiHandler().first << " " << serv.getCgiHandler().second << std::endl;
			std::cout << "        return: " << loc.getReturnCode() << " " << loc.getReturnUrl() << std::endl;
			const std::vector<std::string> &lex = loc.getLimitExcept();
			std::cout << "        limit_except:";
			for (size_t i = 0; i < lex.size(); i++)
				std::cout << " " << lex[i];
			std::cout << std::endl;

			const std::vector<std::string> &am = loc.getAllowMethods();
			std::cout << "        allow_methods:";
			for (size_t i = 0; i < am.size(); i++)
				std::cout << " " << am[i];
			std::cout << std::endl;

			const std::vector<std::string> &lidx = loc.getIndex();
			std::cout << "        index:";
			for (size_t i = 0; i < lidx.size(); i++)
				std::cout << " " << lidx[i];
			std::cout << std::endl;
		}
	}
	if (flag == 1)
		std::cout << GREEN << "\nParser passed." << BLACK << std::endl;
	if (flag == 0)
		std::cout << RED << "\nParser Failed." << BLACK << std::endl;
}

std::string getRecvErrorStr( int err )
{
	switch (err)
	{
		case EAGAIN:
	#if EWOULDBLOCK != EAGAIN
		case EWOULDBLOCK:
	#endif
			return "No data available, try again later";
		case EBADF:
			return "Invalid file descriptor";
		case ECONNREFUSED:
			return "Remote host refused the connection";
		case EFAULT:
			return "Buffer pointer outside valid memory";
		case EINTR:
			return "Interrupted by a signal, retry";
		case EINVAL:
			return "Invalid argument passed";
		case ENOMEM:
			return "Could not allocate memory";
		case ENOTCONN:
			return "Socket is not connected";
		case ENOTSOCK:
			return "File descriptor is not a socket";
		default:
			return "Unknown recv error";
	}
}

std::string getEpollCtlErrorStr( int err )
{
	switch (err)
	{
		case EBADF:
			return "epfd or fd is not a valid file descriptor";
		case EEXIST:
			return "fd is already registered with this epoll instance";
		case EINVAL:
			return "Invalid argument, operation, or event type for epoll_ctl";
		case ELOOP:
			return "Operation would create a circular epoll loop or excessive nesting";
		case ENOENT:
			return "fd is not registered with this epoll instance";
		case ENOMEM:
			return "Insufficient memory to handle the operation";
		case ENOSPC:
			return "max_user_watches limit reached, cannot register fd";
		case EPERM:
			return "fd does not support epoll (e.g. regular file or directory)";
		default:
			return "Unknown epoll_ctl error";
	}
}

std::string getSendErrorStr( int err )
{
	switch (err)
	{
		case EACCES:
			return "Write permission denied on destination socket";
		case EAGAIN:
	#if EWOULDBLOCK != EAGAIN
		case EWOULDBLOCK:
	#endif
			return "Operation would block, try again later";
		case EALREADY:
			return "Another Fast Open is already in progress";
		case EBADF:
			return "Invalid file descriptor";
		case ECONNRESET:
			return "Connection reset by peer";
		case EDESTADDRREQ:
			return "Socket is not connection-mode and no peer address is set";
		case EFAULT:
			return "Invalid user space address specified";
		case EINTR:
			return "Interrupted by a signal before data was sent";
		case EINVAL:
			return "Invalid argument passed";
		case EISCONN:
			return "Socket already connected but a recipient was specified";
		case EMSGSIZE:
			return "Message too large to send atomically";
		case ENOBUFS:
			return "Output queue for the network interface was full";
		case ENOMEM:
			return "No memory available";
		case ENOTCONN:
			return "Socket is not connected";
		case ENOTSOCK:
			return "File descriptor is not a socket";
		case EOPNOTSUPP:
			return "Flags argument inappropriate for this socket type";
		case EPIPE:
			return "Local end has been shut down on this connection";
		default:
			return "Unknown send error";
	}
}

void	logError( std::string errMsg, int flag )
{

}
