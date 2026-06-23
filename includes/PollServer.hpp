#ifndef POLLSERVER_HPP
# define POLLSERVER_HPP

#define MAX_EVENTS 64

#include "WebServ.hpp"
#include "AServer.hpp"

class	PollServer: public AServer
{
	private:
		#ifdef __APPLE__
		int						_epoll_fd;
		struct epoll_event			_event;
		struct epoll_event			_active_events[MAX_EVENTS];
		#endif
		
		PollServer(PollServer const &other);
		PollServer &operator=(PollServer const &other);

		//+ --- Internal functionality ---
		void	closePoll( void );
		void	addClientToMultiplexer( int fd );
		void	removeFdFromMultiplexer( int fd );
		void	watchForRead( int fd );
		void	watchForWrite( int fd );
	public:
		PollServer();
		~PollServer();

		class runtimePollServerException : public std::runtime_error 
		{
			public:
				runtimePollServerException(const char* message);
		};

		//+ --- Configuration and launching
		void	initMultiplexer( void );
		void	addSocketsToMultiplexer( void );
		void	run( void );
};

#endif