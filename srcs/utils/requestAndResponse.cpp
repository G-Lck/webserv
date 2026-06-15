#include "../../includes/utils.hpp"

//+ 1. Pass by reference!
bool request_is_complete(std::map<int, std::string>& client_buffers, int key)
{
    //+ Every valid HTTP request headers section ends with "\r\n\r\n"
    if (client_buffers[key].find("\r\n\r\n") != std::string::npos)
        return true;
    return false;
}

//+ 2. Pass by reference!
std::string extract_request(std::map<int, std::string>& client_buffers, int key)
{
    //+ For testing, just grab whatever is in the buffer
    return client_buffers[key];
}

//+ 3. Pass by value is fine here, it's just one string
std::string process_and_build_response(std::string single_request)
{
    //+ Print what the browser sent us to the terminal
    std::cout << "\n--- NEW REQUEST RECEIVED ---\n" << single_request << "----------------------------\n" << std::endl;

    //+ Hardcode a perfectly valid HTTP/1.1 response
    std::string dummy_html = "<html><body><h1>Hello from epoll!</h1></body></html>";
    std::string dummy_response = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/html\r\n"
                                 "Content-Length: 52\r\n"
                                 "\r\n" + dummy_html;
                                 
    return dummy_response;
}                     

//+ 4. Pass by reference!
void erase_request_from_buffer(std::map<int, std::string>& client_buffers, int key)
{
    //+ For testing, just wipe the entire buffer clean so we don't infinite loop
    client_buffers[key].clear();
}