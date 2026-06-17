#ifndef SERVER_HPP
# define SERVER_HPP

#define MAX_EVENTS 64

#include "WebServ.hpp"
#include "Socket.hpp"
#include "Types.hpp"
#include "GlobalConfig.hpp"

class	Server
{
	private:
		int							_epoll_fd;
		std::map<int, Socket*>		_sockets;
		std::map<int, std::string>	_client_buffers;
		std::map<int, std::string>	_client_responses;
		struct epoll_event			_event;
		struct epoll_event			_active_events[MAX_EVENTS];
		t_virtualServer				_virtualServers;
		t_fdRoute					_fd_to_route;
		
		Server( Server const &other );
		Server &operator=( Server const &other );

		//+ --- Internal functionallity ---
		void	addSocket( Socket *S );
		void	closeSockets( void );
		void	closeEpoll( void );
		bool	fdMatch( int curr );
		bool	addNewClient( int curr_socket_fd );
		void	clientDisconnect( int fd );
		void	setEpollInOut( int fd, int Flag );
		void	handleCompleteRequest( int fd );
		void	readRequest( int fd );
		void	sendResponse( int fd );
	public:
		Server();
		~Server();

		class runtimeServerException : public std::runtime_error 
		{
			public:
				runtimeServerException(const char* message);
		};

		//+ --- Configuration and Launching
		void	configServer( GlobalConfig &config );
		void	epollInit( void );
		void	epollAddSockets( void );
		void	initServer( void );
};

#endif


//* You only instantiate one C++ Server class (your engine).

//* That single engine manages multiple Sockets (the doors). 
//* When a request comes through a socket, your engine reads the Host: 
//* header, checks your list of ServerConfigs, and routes the traffic to 
//* the correct virtual server's rules.

//+ The Phase	What happens with _fd_to_route (also for sockets, this example is specific for clients)
//+ 1. The Birth (addNewClient)	accept() creates fd_client. 
//+ You immediately copy the parent's Host:Port pair into this->_fd_to_route[fd_client].
//+ 2. The Execution (handleCompleteRequest)	
//+ When the HTTP request is fully read, you just look up this->_fd_to_route[fd] 
//+ to get the pair, plug it into this->_virtualServers, and you instantly have 
//+ your matching ServerConfig rules.
//+ 3. The Cleanup (clientDisconnect)	Because you added it to the map, 
//+ you must clean it up. You need to add this->_fd_to_route.erase(fd); when 
//+ the client disconnects so your server does not leak memory over time.
