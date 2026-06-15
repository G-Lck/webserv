#ifndef SERVER_HPP
# define SERVER_HPP

#define MAX_EVENTS 64

#include "WebServ.hpp"

class	Server
{
	private:
		std::vector<Socket*>		_sockets;
		std::map<int, std::string>	_client_buffers;
		std::map<int, std::string>	_client_responses;
		int							_epoll_fd;
		struct epoll_event			_event;
		struct epoll_event			_active_events[MAX_EVENTS];
		Server( Server const &other );
		Server &operator=( Server const &other );
	public:
		Server();
		~Server();

		class runtimeServerException : public std::runtime_error 
		{
			public:
				runtimeServerException(const char* message);
		};

		void	addSocket( Socket *S );
		void	epollInit( void );
		void	epollAddSockets( void );
		void	closeSockets( void );
		void	closeEpoll( void );

		void	initServer( void );
		bool	fdMatch( int curr );
		bool	addNewClient( int curr_socket_fd );
		void	clientDisconnect( int fd );
		void	setEpollInOut( int fd, int Flag );
		void	handleCompleteRequest( int fd );
		void	readRequest( int fd );
		void	sendResponse( int fd );
};

#endif

// {
//     int fd_socket = make_main_socket();  
//     if (fd_socket == -1)
//         return (-1);

//     //+ Persistent containers tying an FD to its leftover inbound and outbound data
//     std::map<int, std::string> client_buffers;
//     std::map<int, std::string> client_responses;

//     //+ 1. Create the epoll instance
//     //~ The '1' is ignored by modern Linux, but must be > 0
//     int epoll_fd = epoll_create(1);
//     if (epoll_fd == -1)
//         return (-1);

//     //+ 2. Add the main socket to epoll to watch for new connections
//     struct epoll_event ev;
//     ev.events = EPOLLIN; 
//     ev.data.fd = fd_socket;
//     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_socket, &ev) == -1)
//         return (-1);

//     //+ Array to hold the events that epoll_wait returns to us
//     struct epoll_event active_events[MAX_EVENTS];

//     while (1)
//     {
//         //+ 3. Wait for events. nfds is the EXACT number of active sockets.
//         int nfds = epoll_wait(epoll_fd, active_events, MAX_EVENTS, -1);
//         if (nfds == -1)
//         {
//             std::cout << "epoll_wait error" << std::endl;
//             continue; 
//         }

//         //+ 4. Iterate ONLY through the active sockets
//         for (int i = 0; i < nfds; ++i)
//         {
//             int current_fd = active_events[i].data.fd;

//             //+ Scenario A: The Main Socket is Ready (New Connection)
//             if (current_fd == fd_socket)
//             {
//                 struct sockaddr_in  addr_client;
//                 socklen_t           addr_size = sizeof(addr_client);
                
//                 int fd_client = accept(fd_socket, (struct sockaddr *)&addr_client, &addr_size);
//                 if (fd_client == -1)
//                     continue;

//                 fcntl(fd_client, F_SETFL, O_NONBLOCK);

//                 //+ Add new client to epoll
//                 struct epoll_event client_ev;
//                 client_ev.events = EPOLLIN; // Watch for incoming HTTP requests
//                 client_ev.data.fd = fd_client;
//                 epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_client, &client_ev);
//             }
//             //+ Scenario B: A Client Socket is Ready to READ (EPOLLIN)
//             else if (active_events[i].events & EPOLLIN)
//             {
//                 char buffer[4096];
//                 int bytes_read = recv(current_fd, buffer, sizeof(buffer), 0);

//                 //+ Cleanup: Client disconnected or error
//                 if (bytes_read <= 0)
//                 {
//                     //~ ALWAYS remove from epoll before closing the FD
//                     epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
//                     close(current_fd);
//                     client_buffers.erase(current_fd);
//                     client_responses.erase(current_fd);
//                 }
//                 else
//                 {
//                     client_buffers[current_fd].append(buffer, bytes_read);

//                     while (request_is_complete(client_buffers[current_fd]))
//                     {
//                         std::string single_request = extract_request(client_buffers[current_fd]);
//                         std::string single_response = process_and_build_response(single_request);
                        
//                         client_responses[current_fd].append(single_response);
//                         erase_request_from_buffer(client_buffers[current_fd]);
//                     }
                    
//                     //+ If we generated any responses, modify epoll to watch for EPOLLOUT
//                     if (!client_responses[current_fd].empty())
//                     {
//                         struct epoll_event mod_ev;
//                         mod_ev.events = EPOLLOUT; 
//                         mod_ev.data.fd = current_fd;
//                         epoll_ctl(epoll_fd, EPOLL_CTL_MOD, current_fd, &mod_ev);
//                     }
//                 }
//             }
//             //+ Scenario C: A Client Socket is Ready to WRITE (EPOLLOUT)
//             else if (active_events[i].events & EPOLLOUT)
//             {
//                 int sent_bytes = send(current_fd, client_responses[current_fd].c_str(), client_responses[current_fd].length(), 0);
                
//                 if (sent_bytes > 0)
//                 {
//                     client_responses[current_fd].erase(0, sent_bytes);
//                 }

//                 //+ If the outbound queue is completely empty, we are done sending.
//                 if (client_responses[current_fd].empty())
//                 {
//                     //+ Switch back to listening for new incoming requests
//                     struct epoll_event mod_ev;
//                     mod_ev.events = EPOLLIN; 
//                     mod_ev.data.fd = current_fd;
//                     epoll_ctl(epoll_fd, EPOLL_CTL_MOD, current_fd, &mod_ev);
//                 }
//             }
//         }
//     }
// }
