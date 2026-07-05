#include "../../includes/utils.hpp"

bool request_is_complete(Client* client)
{
    if (client->getReadBuffer().find("aaa") != std::string::npos)
        return true;
    return false;
}

std::string extract_request(Client* client)
{
    return client->getReadBuffer();
}

std::string process_and_build_response(Client* client)
{
	client->getRequest().printRequest();
	return ("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\nConnection: keep-alive\r\n\r\nOK");
}                     

void erase_request_from_buffer(Client* client)
{
    client->clearReadBuffer();
}