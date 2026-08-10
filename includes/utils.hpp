#ifndef UTILS_HPP
# define UTILS_HPP

#include "WebServ.hpp"
#include "GlobalConfig.hpp"

class Client;

std::string		process_and_build_response( Client *client );                     

void			writeLog( std::string msg, int flag );

std::string		cutString(std::string str, char c);

std::string		getRecvErrorStr( int err );

std::string		getEpollCtlErrorStr( int err );

std::string		getSendErrorStr( int err );

std::string		stringifyConfig(GlobalConfig &config);
   
#endif