#ifndef UTILS_HPP
# define UTILS_HPP

#include "Client.hpp"
#include "WebServ.hpp"

class Client;

bool		request_is_complete( Client *client );

std::string	extract_request( Client *client );

std::string	process_and_build_response( Client *client );                     

void		erase_request_from_buffer( Client *client );
   
#endif