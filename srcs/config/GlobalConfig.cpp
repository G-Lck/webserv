#include "../../includes/WebServ.hpp"

GlobalConfig::GlobalConfig()
{
	std::cout << "GlobalConfig default condstructor called" << std::endl;

	// for now it's hardcoded but we should maybe memset it here
	this->_root = "../../";
	this->_index.push_back("index.html");
	this->_host = "127.0.0.1";
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
		this->_host = other._host;
	}
	return *this;
}

GlobalConfig::~GlobalConfig() {}

const ServerConfig&	GlobalConfig::getServers(unsigned int i)	const
{
	return (this->_servers[i]);
}