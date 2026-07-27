#ifndef ASERVER_HPP
# define ASERVER_HPP

#include "WebServ.hpp"
#include "Socket.hpp"
#include "Client.hpp"
#include "Types.hpp"
#include "GlobalConfig.hpp"
#include "utils.hpp"
#include "Handler.hpp"

class	AServer
{
	protected:
		std::map<int, Socket*>				_sockets;
		std::map<int, Client*>				_clients;
		t_virtualServer						_virtualServers;
		t_fdRoute							_fd_to_route; // This is to give to the handler, to know in which socket the client is connected, to after know in which virtual server i have to go
		//std::map<int, CgiHandler*>			_cgi_handlers;

		AServer(AServer const &other );
		AServer &operator=( AServer const &other );

		//+ --- Internal functionality ---
		void	addSocket(Socket *S);
		void	closeSockets(void);
		void	closeClients(void);
		
		//+ clientelle
		bool	fdMatch(int curr) const;
		bool	addNewClient(int curr_socket_fd);
		void	clientDisconnect(int fd);
		
		//+ read
		void	readRequest(int fd);
		bool	handleCompleteRequest(int fd);
		void	finishRequestCycle(Client* c);
		void	prepareToSend(int fd, Client * c, std::string single_response);
		
		//+ send
		void	sendResponse(int fd);
		
		//+ timeout
		bool	clientTimeout( int fd );
		void	refreshClientTime( int fd );
		void	monitorClients();

		virtual void	initMultiplexer(void) = 0;
		virtual void	addSocketsToMultiplexer(void) = 0;
		virtual void	run(void) = 0;
		virtual bool	addClientToMultiplexer(int fd) = 0;
		virtual void	removeFdFromMultiplexer(int fd) = 0;
		virtual void	watchForRead(int fd) = 0;
		virtual void	watchForWrite(int fd) = 0;

		// CGI support intentionally disabled for now.

	public:
		AServer();
		virtual ~AServer();

		class runtimeAServerException : public std::runtime_error 
		{
			public:
				runtimeAServerException(const char* message);
		};

		//+ --- Configuration and launching
		void	configAServer( GlobalConfig &config );
		void	initAServer(void);
};

#endif