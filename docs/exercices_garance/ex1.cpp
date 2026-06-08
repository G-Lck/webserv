#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int	main()
{
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo; // will point to the results
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	int	sockfd;
	int	new_fd;
	char buf[1];
	int byte_count;

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me

	std::cout << std::endl << "hints" << std::endl;

	std::cout << hints.ai_family << std::endl;
	std::cout << hints.ai_socktype<< std::endl;
	std::cout << hints.ai_flags << std::endl;

	std::cout << std::endl << "servinfo" << std::endl;

	std::cout << servinfo->ai_flags << std::endl;
	std::cout << servinfo->ai_family << std::endl;
	std::cout << servinfo->ai_socktype << std::endl;
	std::cout << servinfo->ai_protocol << std::endl;
	std::cout << servinfo->ai_addrlen << std::endl;

	status = getaddrinfo(NULL, "8080", &hints, &servinfo);

	std::cout << servinfo->ai_flags << std::endl;
	std::cout << servinfo->ai_family << std::endl;
	std::cout << servinfo->ai_socktype << std::endl;
	std::cout << servinfo->ai_protocol << std::endl;
	std::cout << servinfo->ai_addrlen << std::endl;


	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	listen(sockfd, 10);

	addr_size = sizeof their_addr;
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

	//byte_count = recv(new_fd, buf, sizeof buf, 0);
	//printf("recv()'d %d bytes of data in buf\n", byte_count);

	const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 56\r\n\r\n<html><body><h1>Hello!</h1><p>Mon serveur marche</p></body></html>";
	send(new_fd, response, strlen(response), 0);

}