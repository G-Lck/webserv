#include "../../includes/LocationConfig.hpp"

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