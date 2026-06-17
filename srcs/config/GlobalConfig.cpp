#include "../../includes/GlobalConfig.hpp"

// ------ Orthodox ------

GlobalConfig::GlobalConfig()
{
	std::cout << "GlobalConfig default condstructor called" << std::endl;

	// for now it's hardcoded but we should maybe memset it here
	this->_root = "../../";
	this->_index.push_back("index.html");
	ServerConfig	new_server;
	this->_servers.push_back(new_server);
}

GlobalConfig::GlobalConfig(const GlobalConfig& other)
{
	std::cout << "GlobalConfig copy condstructor called" << std::endl;
	*this = other;
}

GlobalConfig& GlobalConfig::operator=(const GlobalConfig& other)
{
	std::cout << "GlobalConfig operator = called" << std::endl;

	if (this != &other) {
		this->_servers = other._servers;
		this->_root = other._root;
		this->_index = other._index;
		this->_error_pages = other._error_pages;
		this->_autoindex = other._autoindex;
		this->_client_max_body_size = other._client_max_body_size;
	}
	return *this;
}

GlobalConfig::~GlobalConfig() {}

// ------ Getters ------

/// @brief Returns the ServerConfig at the specified index
const ServerConfig& GlobalConfig::getServers( size_t i ) const { return this->_servers[i]; }

/// @brief Returns the global default root directory
const std::string& GlobalConfig::getRoot( void ) const { return this->_root; }

/// @brief Returns the global default index files
const std::vector<std::string>& GlobalConfig::getIndex( void ) const { return this->_index; }

/// @brief Returns the global default error pages
const std::vector<std::string>& GlobalConfig::getErrorPages( void ) const { return this->_error_pages; }

/// @brief Returns the global default autoindex setting
bool GlobalConfig::getAutoindex( void ) const { return this->_autoindex; }

/// @brief Returns the global default client max body size limit
int GlobalConfig::getClientMaxBodySize( void ) const { return (int)this->_client_max_body_size; }

/// @brief Returns the amount of serves
size_t	GlobalConfig::serverCount( void ) const { return (this->_servers.size()); }