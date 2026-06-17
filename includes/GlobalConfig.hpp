#ifndef GLOBALCONFIG_HPP
#define GLOBALCONFIG_HPP

#include "WebServ.hpp"

/**
 * @brief the type of privates attributs are maybe a little bit random
 */

class GlobalConfig {

	private:
		std::vector<ServerConfig>	_servers;
		std::string					_root;
		std::vector<std::string>	_index;
		std::vector<std::string>	_error_pages;
		bool						_autoindex;
		int							_client_max_body_size;
		std::string					_host; // not sure that the host should be here

	public:
		GlobalConfig();
		GlobalConfig(const GlobalConfig& other);
		GlobalConfig& operator=(const GlobalConfig& other);
		~GlobalConfig();

		const ServerConfig&	getServers(unsigned int i)	const;
};

#endif