#ifndef TYPES_HPP
#define TYPES_HPP

#include "WebServ.hpp"

class ServerConfig;

/// @brief Maps a physical Host:Port combo to its list of ServerConfigs (Virtual Servers)
typedef std::map< std::pair<std::string, std::string>, std::vector<ServerConfig> >  		t_virtualServer;

/// @brief	Quick lookup map to link an active fd back to its parent Host:Port
///			It paits the fd of the Socket with the Socket host:port
typedef std::map<int, std::pair<std::string, std::string> >                         		t_fdRoute;

/// @brief A simple list holding Host and Port(in that order) pairs, used to read listen directives
typedef std::vector< std::pair<std::string, std::string> >                         			t_port_host;

#endif
