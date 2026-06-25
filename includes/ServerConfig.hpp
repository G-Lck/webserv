#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "WebServ.hpp"
#include "LocationConfig.hpp"

/**
 * @brief the type of privates attributs are maybe a little bit random
 */
 
class ServerConfig {
	private:
		std::vector< std::pair<std::string, std::string> >	_listen;
		std::vector<std::string>							_server_name;
		std::string											_root;
		std::vector<std::string>							_error_page;
		bool												_autoindex;
		std::vector<LocationConfig>							_locations;
		int													_client_max_body_size;
		std::string											_cgi_handler;

	public:
		ServerConfig();
		ServerConfig(const ServerConfig& other);
		ServerConfig& operator=(const ServerConfig& other);
		~ServerConfig();

		const std::pair<std::string, std::string>&	getListen(unsigned int i) const;
		const std::vector< std::pair<std::string, std::string> >& getAllListen(void) const;
};

#endif