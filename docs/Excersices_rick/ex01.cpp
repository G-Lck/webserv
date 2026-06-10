#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
int main(void)
{
    // address struct for our server
    struct sockaddr_in myaddr;

    std::memset(&myaddr, 0, sizeof myaddr);
    myaddr.sin_family = AF_INET;        // IPv4
    myaddr.sin_port = htons(8080);      // port 8080, converted to network byte order

    // convert IP string to binary and store in myaddr
    int retInet = inet_pton(AF_INET, "127.0.0.1", &(myaddr.sin_addr));
    if (retInet == -1) { std::cout << "inet error" << std::endl; return (-1); }
    if (retInet == 0)  { std::cout << "invalid address" << std::endl; return (-1); }

    // create the socket (IPv4, TCP)
    int socketfd = socket(PF_INET, SOCK_STREAM, 0);
    if (socketfd == -1) { std::cout << "socket error" << std::endl; return (-1); }

        // Fix 2: Tell the OS to let us reuse port 8080 immediately after a restart
    int opt = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cout << "setsockopt error" << std::endl;
        return (-1);
    }

    // bind socket to the address/port we configured
    if (bind(socketfd, (struct sockaddr*)&myaddr, sizeof myaddr) == -1)
    { std::cout << "bind error" << std::endl; return (-1); }

    // start listening, queue up to 10 pending connections
    if (listen(socketfd, 10) == -1)
    { std::cout << "listen error" << std::endl; return (-1); }

    struct sockaddr_in their_addr;              // will store the client's address
        // *Always memset address
        std::memset(&their_addr, 0, sizeof their_addr);
    socklen_t addr_size = sizeof their_addr;

    while (1)
    {
        // block until a client connects, returns a new fd for that connection
        int newfd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);
        if (newfd == -1)
            continue;

        // read the client's HTTP request
        char buf[4096];
        int bytes = recv(newfd, buf, sizeof buf - 1, 0);
                if(bytes != -1)
                {
                        buf[bytes] = '\0';
                        std::cout << buf << std::endl;
                }

        // send a minimal HTTP response the browser can understand
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nHello World";
        send(newfd, response.c_str(), response.size(), 0);

        // close the client connection (socketfd keeps listening)
        close(newfd);
    }
}