#ifndef SOCKET_HPP
# define SOCKET_HPP

#define BACKLOG 10

//* FOR NOW IM USING THIS, UNTIL I HAVE YOUR CONFIG, WE CAN TALK ABOUT WHAT WE NEED AFTER
typedef struct s_config
{
    std::string listen;					//8002
    std::string	server_name;			//localhost;
    std::string	host;					//127.0.0.1;
    std::string	root;					//docs/fusion_web/;
//	long		client_max_body_size; 	//3000000;
    std::string	index;					// ../../index.html;
    std::string	error_page;				//404 error_pages/404.html;
}	t_config;

class	Socket
{
	private:
		int					_status; 
		int					_fd;
		struct addrinfo		_hints;
		struct addrinfo*	_socket;
		t_config			_config;
		Socket( Socket const &other );
		Socket &operator=( Socket const &other );
	public:
		Socket();
		~Socket();

		class runtimeSocketException : public std::runtime_error 
		{
			public:
				runtimeSocketException(const char* message);
		};

		//+ -----------
		//* To errase after?
		void	setConfig();
		//* To errase after?
		void	socketGetAddrInfo( void );
		void	socketCall( void );
		void	socketOpt( void );
		void	socketBind( void );
		void	socketFreeAddrInfo( void );
		void	socketListen( void );
		void	socketSetNonBlock( void );
		void	makeSocket( void );

		//+ --- Getters / Setters / Helpers ---
		int		getFd( void );
		void	setFd( int n );
		void	closeFd( void );
};

#endif