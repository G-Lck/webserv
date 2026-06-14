#ifndef UTILS_HPP
# define UTILS_HPP

#include "WebServ.hpp"

int			request_is_complete(std::map<int, std::string> *client_buffers);

std::string	extract_request(std::map<int, std::string> *client_buffers);

std::string	process_and_build_response(std::string single_request);                     

void		erase_request_from_buffer(std::map<int, std::string> *client_buffers);
   
#endif