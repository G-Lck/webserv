#include "../../includes/utils.hpp"
#include "../../includes/Client.hpp"

std::string process_and_build_response(Client* client)
{
	client->getRequest().printRequest();
	return ("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\nConnection: keep-alive\r\n\r\nOK");
}

// updateTime after when? after we recieve a request and when we finish sending the respose.

// get the http request already parsed
// check on path and do the 2 cases 
// try catch the selected case
// try recieves the ok response
// catch recieves the status code error

/*
if (cgi)
	CGIHandler handler(c, this->_virtualServer, this->_fd_to_route);
else
	staticHandler handler;
handler.run();
*/