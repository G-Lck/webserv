#include "../../includes/ServerConfig.hpp"

ServerConfig::ServerConfig()
{
	std::cout << "ServerConfig default condstructor called" << std::endl;

	this->_listen.push_back(std::make_pair("127.0.0.1", "8080"));
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

const std::pair<std::string, std::string>& ServerConfig::getListen(unsigned int i) const { return this->_listen.at(i); }

/// @brief Returns all ports for this Server
/// @return Returns a std::vector< std::pair<std::string, std::string> > containing the ports
///			The first member of the pair is the host, and the second is the port
const std::vector< std::pair<std::string, std::string> >	&ServerConfig::getAllListen(void) const { return (this->_listen); }
