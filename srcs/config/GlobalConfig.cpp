#include "../../includes/GlobalConfig.hpp"

// ------ Orthodox ------

GlobalConfig::GlobalConfig() : _autoindex(false), _client_max_body_size(MAX_BODY) {}

GlobalConfig::GlobalConfig(const GlobalConfig& other) { (void)other; }

GlobalConfig& GlobalConfig::operator=(const GlobalConfig& other) { (void)other; return *this; }

GlobalConfig::~GlobalConfig() {}

// ------ Getters ------

/// @brief Returns the ServerConfig at the specified index
const ServerConfig& GlobalConfig::getServers( size_t i ) const { return this->_servers[i]; }

/// @brief Returns the global default root directory
const std::string& GlobalConfig::getRoot(void) const { return this->_root; }

/// @brief Returns the global default index files
const std::vector<std::string>& GlobalConfig::getIndex(void) const { return this->_index; }

/// @brief Returns the global default error pages
const std::map<int, std::string>& GlobalConfig::getErrorPages( void ) const { return this->_error_pages; }

/// @brief Returns the global default autoindex setting
bool GlobalConfig::getAutoindex(void) const { return this->_autoindex; }

/// @brief Returns the global default client max body size limit
int GlobalConfig::getClientMaxBodySize(void) const { return (int)this->_client_max_body_size; }

/// @brief Returns the amount of serves
size_t	GlobalConfig::serverCount( void ) const { return (this->_servers.size()); }

/// @brief Returns the Server by reference at index
ServerConfig& GlobalConfig::getServersAt(int index) { return this->_servers[index]; }

// ------ Setters ------

/// @brief Adds a server to the server list
void GlobalConfig::addServer( ServerConfig newServ ) { _servers.push_back(newServ); }

/// @brief Sets the root directory
void GlobalConfig::setRoot( const std::string &root ) { _root = root; }

/// @brief Sets the index files
void GlobalConfig::addIndex( const std::string &index ) { this->_index.push_back(index); }

/// @brief Sets the error pages
void GlobalConfig::addErrorPage( int code, const std::string &path ) { this->_error_pages[code] = path; }

/// @brief Sets the autoindex
void GlobalConfig::setAutoindex( bool autoindex ) { _autoindex = autoindex; }

/// @brief Sets the client max body size
void GlobalConfig::setClientMaxBodySize( int size ) { _client_max_body_size = size; }
