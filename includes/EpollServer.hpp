#ifndef EPOLLSERVER_HPP
# define EPOLLSERVER_HPP

#define MAX_EVENTS 64

#include "WebServ.hpp"
#include "AServer.hpp"

class	EpollServer: public AServer
{
	private:
		#ifdef __linux__
		int							_epoll_fd;
		struct epoll_event			_event;
		struct epoll_event			_active_events[MAX_EVENTS];
		#endif
		
		EpollServer( EpollServer const &other );
		EpollServer &operator=( EpollServer const &other );

		//+ --- Internal functionality ---
		void	closeEpoll(void);
		bool	addClientToMultiplexer( int fd );
		void	removeFdFromMultiplexer( int fd );
		void	watchForRead( int fd );
		void	watchForWrite( int fd );
	public:
		EpollServer();
		~EpollServer();

		class runtimeEpollServerException : public std::runtime_error 
		{
			public:
				runtimeEpollServerException(const char* message);
		};

		//+ Getters
		const Client	&getClient(int fd) const;
		const Socket	&getSocket(int fd) const;

		//+ --- Configuration and launching
		void	initMultiplexer(void);
		void	addSocketsToMultiplexer(void);
		void	run(void);
};

#endif
