#include "../../includes/ServerConfig.hpp"

// ------- Orthodox -------

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

// ------- Setters -------

/// @brief Returns the listen pair at index i
const std::pair<std::string, std::string>& ServerConfig::getListen(unsigned int i) const { return this->_listen.at(i); }

/// @brief Returns all ports for this Server
/// @return Returns a std::vector< std::pair<std::string, std::string> > containing the ports
///			The first member of the pair is the host, and the second is the port
const std::vector< std::pair<std::string, std::string> >	&ServerConfig::getAllListen( void ) const { return (this->_listen); }

/// @brief Returns the server names
const std::vector<std::string> &ServerConfig::getServerName( void ) const
{ return _server_name; }

/// @brief Returns the root directory
const std::string &ServerConfig::getRoot( void ) const
{ return _root; }

/// @brief Returns the error pages
const std::vector<std::string> &ServerConfig::getErrorPage( void ) const
{ return _error_page; }

/// @brief Returns the autoindex
bool ServerConfig::getAutoindex( void ) const
{ return _autoindex; }

/// @brief Returns the locations
const std::vector<LocationConfig> &ServerConfig::getLocations( void ) const
{ return _locations; }

/// @brief Returns the client max body size
int ServerConfig::getClientMaxBodySize( void ) const
{ return _client_max_body_size; }

/// @brief Returns the cgi handler
const std::string &ServerConfig::getCgiHandler( void ) const
{ return _cgi_handler; }

// ------- Getters -------

/// @brief Sets the listen pair
void ServerConfig::addListen(const std::string &host, const std::string &port)
{ _listen.push_back(std::make_pair(host, port)); }

/// @brief Sets the server name
void ServerConfig::addServerName(const std::string &name)
{ _server_name.push_back(name); }

/// @brief Sets the root directory
void ServerConfig::setRoot(const std::string &root)
{ _root = root; }

/// @brief Adds an error page
void ServerConfig::addErrorPage(const std::string &error_page)
{ _error_page.push_back(error_page); }

/// @brief Sets the autoindex
void ServerConfig::setAutoindex(bool autoindex)
{ _autoindex = autoindex; }

/// @brief Adds a location
void ServerConfig::addLocation(const LocationConfig &location)
{ _locations.push_back(location); }

/// @brief Sets the client max body size
void ServerConfig::setClientMaxBodySize(int size)
{ _client_max_body_size = size; }

/// @brief Sets the cgi handler
void ServerConfig::setCgiHandler(const std::string &cgi_handler)
{ _cgi_handler = cgi_handler; }