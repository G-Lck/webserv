#ifndef GLOBALCONFIG_HPP
#define GLOBALCONFIG_HPP

#include "ServerConfig.hpp"

class GlobalConfig {
	private:
		std::vector<ServerConfig>	_servers;

	public:
		GlobalConfig();
		GlobalConfig(const GlobalConfig& other);
		GlobalConfig& operator=(const GlobalConfig& other);
		~GlobalConfig();
};

#endif