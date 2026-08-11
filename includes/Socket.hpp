#ifndef SOCKET_HPP
# define SOCKET_HPP

#define BACKLOG 100 //+ Check socketListen()

#include "WebServ.hpp"

class	Socket
{
	private:
		int									_status; 
		int									_fd;
		std::pair<std::string, std::string>	_host_port;
		struct addrinfo						_hints;
		struct addrinfo*					_socket;

		Socket( Socket const &other );
		Socket &operator=( Socket const &other );

		//+ --- Main Functionality ---
		void	socketGetAddrInfo( std::string host, std::string port );
		void	socketCall(void);
		void	socketOpt(void);
		void	socketBind(void);
		void	socketFreeAddrInfo(void);
		void	socketListen(void);
		void	socketSetNonBlock(void);
	public:
		Socket();
		~Socket();

		class runtimeSocketException : public std::runtime_error 
		{
			public:
				runtimeSocketException(const char* message);
		};

		//+ --- Main executor ---
		void	makeSocket( std::string host, std::string port );

		//+ --- Getters / Setters / Helpers ---
		int									getFd(void) const;
		void								setFd( int n );
		std::pair<std::string, std::string> getHostPort() const;
		void								closeFd(void);
};

std::ostream &operator<<( std::ostream &out, Socket const &s );

#endif