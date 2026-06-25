#ifndef SOCKET_HPP
# define SOCKET_HPP

#define BACKLOG 10 //+ Check socketListen()

#include "WebServ.hpp"

class	Socket
{
	private:
		int					_status; 
		int					_fd;
		struct addrinfo		_hints;
		struct addrinfo*	_socket;

		Socket( Socket const &other );
		Socket &operator=( Socket const &other );

		//+ --- Main Functionality ---
		void	socketGetAddrInfo( std::string port, std::string host );
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
		void	makeSocket( std::string port, std::string host );

		//+ --- Getters / Setters / Helpers ---
		int		getFd(void);
		void	setFd( int n );
		void	closeFd(void);
};

#endif