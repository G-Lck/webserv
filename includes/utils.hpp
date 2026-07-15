#ifndef UTILS_HPP
# define UTILS_HPP

#include "WebServ.hpp"
#include "Client.hpp"
#include "Log.hpp"
#include "GlobalConfig.hpp"


std::string		process_and_build_response( Client *client );                     

void			writeLog( std::string msg, int flag );
   
std::string		getRecvErrorStr( int err );

std::string		getEpollCtlErrorStr( int err );

std::string		getSendErrorStr( int err );

std::string		stringifyConfig(GlobalConfig &config, int flag);
   
#endif