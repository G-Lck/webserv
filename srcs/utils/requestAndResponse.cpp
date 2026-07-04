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
    std::string single_request = client->getWriteBuffer();
    std::cout << "\n--- NEW REQUEST RECEIVED ---\n" << single_request << "----------------------------\n" << std::endl;

    std::string dummy_html = "<html><body><h1>Hello from Client Object!</h1></body></html>";
    std::string dummy_response = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/html\r\n"
                                 "Content-Length: 58\r\n"
                                 "\r\n" + dummy_html;
                                 
    return dummy_response;
}                     

void erase_request_from_buffer(Client* client)
{
    client->clearReadBuffer();
}