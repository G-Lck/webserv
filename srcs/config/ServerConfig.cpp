#include "../../includes/WebServ.hpp"

ServerConfig::ServerConfig()
{
	std::cout << "ServerConfig default condstructor called" << std::endl;

	this->_listen.push_back("8080");
	this->_server_name.push_back("localhost");
	LocationConfig	new_location;
	this->_locations.push_back(new_location);
}

ServerConfig::ServerConfig(const ServerConfig& other)
{
	std::cout << "ServerConfig copy condstructor called" << std::endl;
	*this = other;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	std::cout << "ServerConfig operator = copy called" << std::endl;
	if (this != &other) {
		this->_listen = other._listen;
		this->_server_name = other._server_name;
		this->_root = other._root;
		this->_error_page = other._error_page;
		this->_autoindex = other._autoindex;
		this->_locations = other._locations;
		this->_client_max_body_size = other._client_max_body_size;
		this->_cgi_handler = other._cgi_handler;
	}
	return *this;
}

ServerConfig::~ServerConfig() {}

const std::string& ServerConfig::getListen(unsigned int i) const
{
    return this->_listen.at(i);
}