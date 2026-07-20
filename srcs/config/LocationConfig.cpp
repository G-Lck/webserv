#include "../../includes/LocationConfig.hpp"

// ------ Orthodox ------

LocationConfig::LocationConfig() : _autoindex(false), _client_max_body_size(0), _return_code(0) {}

LocationConfig::LocationConfig(const LocationConfig &other) :
	_root(other._root),
	_path(other._path),
	_autoindex(other._autoindex),
	_client_max_body_size(other._client_max_body_size),
	_index(other._index),
	_allow_methods(other._allow_methods),
	_error_pages(other._error_pages),
	_cgi_handler(other._cgi_handler),
	_limit_except(other._limit_except),
	_return_url(other._return_url),
	_return_code(other._return_code),
	_upload_path(other._upload_path)
{}

LocationConfig& LocationConfig::operator=(const LocationConfig& other)
{
	if (this != &other) {
		this->_root = other._root;
		this->_path = other._path; 
		this->_autoindex = other._autoindex;
		this->_client_max_body_size = other._client_max_body_size;
		this->_index = other._index;
		this->_allow_methods = other._allow_methods;
		this->_error_pages = other._error_pages;
		this->_cgi_handler = other._cgi_handler;
		this->_limit_except = other._limit_except;
		this->_return_url = other._return_url;
		this->_return_code = other._return_code;
		this->_upload_path = other._upload_path;
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

/// @brief Sets the index files
void LocationConfig::addIndex( const std::string &index ) { this->_index.push_back(index); }

/// @brief Sets the allowed methods
void LocationConfig::setAllowMethods(const std::vector<std::string> &methods) { _allow_methods = methods; }

/// @brief Adds a single error page to the map
void LocationConfig::addErrorPage(int code, const std::string &path) { _error_pages[code] = path; }

/// @brief Sets the cgi handler
void LocationConfig::setCgiHandler(const std::pair<std::string, std::string> &cgi_handler) { _cgi_handler = cgi_handler; }

/// @brief Sets the limit except
void LocationConfig::setLimitExcept(const std::vector<std::string> &limit_except) { _limit_except = limit_except; }

/// @brief Sets the return code and url
void LocationConfig::setReturn(int code, const std::string &url) { _return_code = code; _return_url = url; }

/// @brief Sets the upload directory path
void LocationConfig::setUploadPath(const std::string &upload_path) { _upload_path = upload_path; }

/// @brief Sets the error pages
void LocationConfig::setErrorPages(const std::map<int, std::string> &error_pages) { this->_error_pages = error_pages; }

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

/// @brief Returns the allowed methods
const std::vector<std::string> &LocationConfig::getAllowMethods( void ) const { return _allow_methods; }

/// @brief Returns the error pages map
const std::map<int, std::string> &LocationConfig::getErrorPages( void ) const { return _error_pages; }

/// @brief Returns the cgi handler
const std::pair<std::string, std::string> &LocationConfig::getCgiHandler( void ) const { return _cgi_handler; }

/// @brief Returns the limit except
const std::vector<std::string> &LocationConfig::getLimitExcept( void ) const { return _limit_except; }

/// @brief Returns the return code
int LocationConfig::getReturnCode( void ) const { return _return_code; }

/// @brief Returns the return url
const std::string &LocationConfig::getReturnUrl( void ) const { return _return_url; }

/// @brief Returns the upload directory path
const std::string &LocationConfig::getUploadPath( void ) const { return _upload_path; }