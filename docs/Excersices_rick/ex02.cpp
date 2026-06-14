// --- C++ Standard Core ---
#include <iostream>     // std::cout, std::cerr
#include <string>       // std::string
#include <vector>       // std::vector (perfect for managing your pollfd roster)
#include <cstring>      // std::memset(), std::strlen()
#include <utility>		// std::pair
#include <map>			// std::map

// --- POSIX Sockets & Network ---
#include <sys/socket.h> // socket(), bind(), listen(), accept(), send(), recv(), setsockopt()
#include <netdb.h>      // getaddrinfo(), freeaddrinfo(), struct addrinfo
#include <netinet/in.h> // struct sockaddr_in, htons(), htonl(), ntohs(), ntohl()
#include <sys/epoll.h> // epoll()

// --- POSIX Multiplexing & System Calls ---
#include <poll.h>       // poll(), struct pollfd, POLLIN, POLLOUT
#include <fcntl.h>      // fcntl(), F_SETFL, O_NONBLOCK
#include <unistd.h>     // close(), read(), write(), fork(), execve(), pipe(), dup2()

// --- POSIX File System & CGI ---
#include <sys/stat.h>   // stat() (to check if an HTML file exists before serving)
#include <dirent.h>     // opendir(), readdir(), closedir() (for autoindex)

typedef struct s_config
{
    std::string listen;			//8002
    std::string	server_name;	//localhost;
    std::string	host;			//127.0.0.1;
    std::string	root;			//docs/fusion_web/;
//	long		client_max_body_size; //3000000;
    std::string	index;			// ../../index.html;
    std::string	error_page;		//404 error_pages/404.html;
}	t_config;

/// @brief Makes a new socket
/// @return Returrns socket fd
int	make_socket()
{
	//+ Fill my congif file struct
	t_config config;
	config.listen = 8002;
	config.server_name = "localhost";
	config.host = "127.0.0.1";
	config.root = "../../";
	config.index = "index.html";

	//+ Socket call pre-setup
	int					status; 
	int					fd_socket;	// Socket file descriptor
	struct addrinfo		hints;		// Hints needed by getaddrinfo() for the creation of the socket
	struct addrinfo		*Socket;	// Pointer to the socket created by socket()

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me

	status = getaddrinfo(NULL, config.listen.c_str(), &hints, &Socket);
	//* CHECK HERE ALL RETURN ERRORS (TO CATCH)
	if (status != 0)
	{
        std::cout << "getaddrinfo error" << std::endl;
        return (-1);
    }

	//+ Create Socket
	fd_socket = socket(Socket->ai_family, Socket->ai_socktype, Socket->ai_protocol);
	if (fd_socket == -1)
    {
        std::cout << "socket error" << std::endl;
        return (-1);
    }

	//+ Avoids "Address already in use" error.
	//+ In case of server stopping, this lets bind to assing the same port from before
	//+ This is because if the server stops, the OS keeps this port for 60 seconds after. With this we avoid that
    int opt = 1;
    if (setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) 
    {
        std::cout << "setsockopt error" << std::endl;
        return (-1);
    }

    //+ Assign the IP and Port to fd_socket
    bind(fd_socket, Socket->ai_addr, Socket->ai_addrlen);
    { 
        std::cout << "bind error" << std::endl; 
        return (-1); 
    }

	//+ Free the linked list memory now, we don't need it after bind()
    freeaddrinfo(Socket);

	// + listen() tells the OS to start accepting incoming connections. 
    // + The 10 is the backlog: how many incoming connections can wait in line at once.
    if (listen(fd_socket, 10) == -1)
    { 
        std::cout << "listen error" << std::endl; 
        return (-1); 
    }

	//+ Apply non block to main socket
	//~It forces functions like recv() and accept() to return an error immediately if there is no data, rather than freezing the program.
	fcntl(fd_socket, F_SETFL, O_NONBLOCK);
}
#define MAX_EVENTS 64

int main()
{
    int fd_socket = make_socket();  
    if (fd_socket == -1)
        return (-1);

    //+ Persistent containers tying an FD to its leftover inbound and outbound data
    std::map<int, std::string> client_buffers;
    std::map<int, std::string> client_responses;

    //+ 1. Create the epoll instance
    //~ The '1' is ignored by modern Linux, but must be > 0
    int epoll_fd = epoll_create(1);
    if (epoll_fd == -1)
        return (-1);

    //+ 2. Add the main socket to epoll to watch for new connections
    struct epoll_event ev;
    ev.events = EPOLLIN; 
    ev.data.fd = fd_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_socket, &ev) == -1)
        return (-1);

    //+ Array to hold the events that epoll_wait returns to us
    struct epoll_event active_events[MAX_EVENTS];

    while (1)
    {
        //+ 3. Wait for events. nfds is the EXACT number of active sockets.
        int nfds = epoll_wait(epoll_fd, active_events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            std::cout << "epoll_wait error" << std::endl;
            continue; 
        }

        //+ 4. Iterate ONLY through the active sockets
        for (int i = 0; i < nfds; ++i)
        {
            int current_fd = active_events[i].data.fd;

            //+ Scenario A: The Main Socket is Ready (New Connection)
            if (current_fd == fd_socket)
            {
                struct sockaddr_in  addr_client;
                socklen_t           addr_size = sizeof(addr_client);
                
                int fd_client = accept(fd_socket, (struct sockaddr *)&addr_client, &addr_size);
                if (fd_client == -1)
                    continue;

                fcntl(fd_client, F_SETFL, O_NONBLOCK);

                //+ Add new client to epoll
                struct epoll_event client_ev;
                client_ev.events = EPOLLIN; // Watch for incoming HTTP requests
                client_ev.data.fd = fd_client;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_client, &client_ev);
            }
            //+ Scenario B: A Client Socket is Ready to READ (EPOLLIN)
            else if (active_events[i].events & EPOLLIN)
            {
                char buffer[4096];
                int bytes_read = recv(current_fd, buffer, sizeof(buffer), 0);

                //+ Cleanup: Client disconnected or error
                if (bytes_read <= 0)
                {
                    //~ ALWAYS remove from epoll before closing the FD
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                    close(current_fd);
                    client_buffers.erase(current_fd);
                    client_responses.erase(current_fd);
                }
                else
                {
                    client_buffers[current_fd].append(buffer, bytes_read);

                    while (request_is_complete(client_buffers[current_fd]))
                    {
                        std::string single_request = extract_request(client_buffers[current_fd]);
                        std::string single_response = process_and_build_response(single_request);
                        
                        client_responses[current_fd].append(single_response);
                        erase_request_from_buffer(client_buffers[current_fd]);
                    }
                    
                    //+ If we generated any responses, modify epoll to watch for EPOLLOUT
                    if (!client_responses[current_fd].empty())
                    {
                        struct epoll_event mod_ev;
                        mod_ev.events = EPOLLOUT; 
                        mod_ev.data.fd = current_fd;
                        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, current_fd, &mod_ev);
                    }
                }
            }
            //+ Scenario C: A Client Socket is Ready to WRITE (EPOLLOUT)
            else if (active_events[i].events & EPOLLOUT)
            {
                int sent_bytes = send(current_fd, client_responses[current_fd].c_str(), client_responses[current_fd].length(), 0);
                
                if (sent_bytes > 0)
                {
                    client_responses[current_fd].erase(0, sent_bytes);
                }

                //+ If the outbound queue is completely empty, we are done sending.
                if (client_responses[current_fd].empty())
                {
                    //+ Switch back to listening for new incoming requests
                    struct epoll_event mod_ev;
                    mod_ev.events = EPOLLIN; 
                    mod_ev.data.fd = current_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, current_fd, &mod_ev);
                }
            }
        }
    }
}
