#include "../../includes/LocationConfig.hpp"

// ------ Orthodox ------

LocationConfig::LocationConfig()
{
	std::cout << "LocationConfig default constructor called" << std::endl;
}

LocationConfig::LocationConfig(const LocationConfig& other)
{
	std::cout << "LocationConfig copy condstructor called" << std::endl;
	*this = other;
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other)
{
	std::cout << "LocationConfig operator = called" << std::endl;
	if (this != &other) {
		this->_root = other._root;
		this->_autoindex = other._autoindex;
		this->_client_max_body_size = other._client_max_body_size;
		this->_index = other._index;
		this->_error_pages = other._error_pages;
		this->_cgi_handler = other._cgi_handler;
		this->_limit_except = other._limit_except;
		this->_return = other._return;
	}
	return *this;
}

LocationConfig::~LocationConfig() {}

// ------ Setters ------

/// @brief Sets the root directory
void LocationConfig::setRoot(const std::string &root) { _root = root; }

/// @brief Sets the path
void LocationConfig::setPath(const std::string &path) { _path = path; }

/// @brief Sets the autoindex
void LocationConfig::setAutoindex(bool autoindex) { _autoindex = autoindex; }

/// @brief Sets the client max body size
void LocationConfig::setClientMaxBodySize(int size) { _client_max_body_size = size; }

/// @brief Sets the index files
void LocationConfig::setIndex(const std::vector<std::string> &index) { _index = index; }

/// @brief Sets the error pages
void LocationConfig::setErrorPages(const std::vector<std::string> &error_pages) { _error_pages = error_pages; }

/// @brief Sets the cgi handler
void LocationConfig::setCgiHandler(const std::string &cgi_handler) { _cgi_handler = cgi_handler; }

/// @brief Sets the limit except
void LocationConfig::setLimitExcept(const std::string &limit_except) { _limit_except = limit_except; }

/// @brief Sets the return
void LocationConfig::setReturn(const std::string &ret) { _return = ret; }

// ------ Getters ------

/// @brief Returns the root directory
const std::string &LocationConfig::getRoot( void ) const { return _root; }

/// @brief Returns the path
const std::string &LocationConfig::getPath( void ) const { return _path; }

/// @brief Returns the autoindex
bool LocationConfig::getAutoindex( void ) const { return _autoindex; }

/// @brief Returns the client max body size
int LocationConfig::getClientMaxBodySize( void ) const { return _client_max_body_size; }

/// @brief Returns the index files
const std::vector<std::string> &LocationConfig::getIndex( void ) const { return _index; }

/// @brief Returns the error pages
const std::vector<std::string> &LocationConfig::getErrorPages( void ) const { return _error_pages; }

/// @brief Returns the cgi handler
const std::string &LocationConfig::getCgiHandler( void ) const { return _cgi_handler; }

/// @brief Returns the limit except
const std::string &LocationConfig::getLimitExcept( void ) const { return _limit_except; }

/// @brief Returns the return
const std::string &LocationConfig::getReturn( void ) const { return _return; }