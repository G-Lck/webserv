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
#include <arpa/inet.h>  // inet_pton() (if you still need manual IP conversion)

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

int	main()
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

	//+ Persistent containers tying an FD to its leftover inbound and outbound data
    std::map<int, std::string> client_buffers;    // Data received but not fully parsed
    std::map<int, std::string> client_responses;  // Responses built but not fully sent

    //+ Container for poll() to monitor
    std::vector<struct pollfd> fds;

    //+ Add the main socket to the container BEFORE the loop
    struct pollfd main_socket;
    main_socket.fd = fd_socket;
    main_socket.events = POLLIN; 
    main_socket.revents = 0;
    fds.push_back(main_socket);

    while (1)
    {
        //+ Wait until an event occurs on ANY socket in our vector
        //* &fds[0] is the standard way to pass vector data to a C function in C++98
        if (poll(&fds[0], fds.size(), -1) == -1)
        {
            std::cout << "poll error" << std::endl;
            continue; 
        }

        //+ Iterating Through the Results
        for (size_t i = 0; i < fds.size(); ++i)
        {
            //+ If revents is 0, nothing happened on this specific socket
            if (fds[i].revents == 0)
                continue;

            //+ Is the socket ready to be read?
            if (fds[i].revents & POLLIN)
            {
                //+ 6. Scenario A: The Main Socket is Ready (New Connection)
                if (fds[i].fd == fd_socket)
                {
                    struct sockaddr_in  addr_client;
                    socklen_t           addr_size = sizeof(addr_client);
                    
                    int fd_client = accept(fd_socket, (struct sockaddr *)&addr_client, &addr_size);
                    if (fd_client == -1)
                        continue;

                    //+ Apply non-block to this new client
                    fcntl(fd_client, F_SETFL, O_NONBLOCK);

                    //+ Add to Container
                    struct pollfd new_client;
                    new_client.fd = fd_client;
                    new_client.events = POLLIN; 
                    new_client.revents = 0;
                    fds.push_back(new_client);
                }
                //+ Scenario B: A Client Socket is Ready (Incoming Request)
                else
                {
                    char buffer[4096];
                    int bytes_read = recv(fds[i].fd, buffer, sizeof(buffer), 0);

                    //+ Cleanup: Client disconnected or error
                    if (bytes_read <= 0)
                    {
                        close(fds[i].fd);
                        client_buffers.erase(fds[i].fd);   //~ Prevent memory leaks
                        client_responses.erase(fds[i].fd); //~ Prevent memory leaks
                        fds.erase(fds.begin() + i);
                        --i; //~ Adjust iterator since vector shrank
                    }
                    else
                    {
                        //+ 1. Append raw bytes to this client's persistent string
                        client_buffers[fds[i].fd].append(buffer, bytes_read);

                        //+ 2. Process ALL complete requests trapped in the string
                        while (request_is_complete(client_buffers[fds[i].fd]))
                        {
                            //+ Extract, parse, and build response (You implement these)
                            std::string single_request = extract_request(client_buffers[fds[i].fd]);
                            std::string single_response = process_and_build_response(single_request);

                            //+ Append response to OUTBOUND queue for this client
                            client_responses[fds[i].fd].append(single_response);

                            //+ Erase ONLY the parsed request from the INBOUND buffer
                            erase_request_from_buffer(client_buffers[fds[i].fd]);
                        }
                        
                        //+ If we generated any responses, switch to POLLOUT
                        if (!client_responses[fds[i].fd].empty())
                        {
                            fds[i].events = POLLOUT;
                        }
                    }
                }
            }
            //+ Scenario C: A Client Socket is Ready to WRITE (POLLOUT)
            else if (fds[i].revents & POLLOUT)
            {
                //+ Push our queued HTTP responses to the client
                int sent_bytes = send(fds[i].fd, client_responses[fds[i].fd].c_str(), client_responses[fds[i].fd].length(), 0);
                
                if (sent_bytes > 0)
                {
                    //+ Erase only the bytes we actually managed to send
                    client_responses[fds[i].fd].erase(0, sent_bytes);
                }

                //+ If the outbound queue is completely empty, we are done sending.
                if (client_responses[fds[i].fd].empty())
                {
                    //+ Switch back to listening for new incoming requests
                    fds[i].events = POLLIN; 
                }
            }
        }
    }
}
