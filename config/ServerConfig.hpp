#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "LocationConfig.hpp"
#include <algortihm>

/**
 * @brief the type of privates attributs are maybe a little bit random
 */
 
class ServerConfig {
	private:
		std::vector<std::string>	_listen;
		std::vector<std::string>	_server_name;
		std::string					_root;
		std::vector<std::string>	_error_page;
		bool						_autoindex;
		std::vector<LocationConfig>	_locations;
		int							_client_max_body_size;
		std::string					_cgi_handler;

	public:
		ServerConfig();
		ServerConfig(const ServerConfig& other);
		ServerConfig& operator=(const ServerConfig& other);
		~ServerConfig();
};

#endif