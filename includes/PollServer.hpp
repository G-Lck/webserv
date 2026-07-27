#ifndef POLLSERVER_HPP
# define POLLSERVER_HPP

#include "WebServ.hpp"
#include "AServer.hpp"

class	PollServer: public AServer
{
	private:
		std::vector<struct pollfd>	_pollfds;
		std::map<int, size_t>		_fd_to_poll_index;
		
		PollServer(PollServer const &other);
		PollServer &operator=(PollServer const &other);

		//+ --- Internal functionality ---
		void	clearPollState(void);
		bool	addFdToMultiplexer( int fd );
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
		void	initMultiplexer(void);
		void	addSocketsToMultiplexer(void);
		void	run(void);
};

#endif