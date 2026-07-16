#include "../../includes/AHandler.hpp"

AHandler::AHandler() {}

AHandler::AHandler(const AHandler& other) { *this = other; }

AHandler& AHandler::operator=(const AHandler& other) {
	if (this != &other)
	{
		this->_client = other.client;
		this->_http_request = other._http_request;
		this->_http_response = other._http_response;
		this->_global_config = other._global_config;
		this->_location = other._location;
		this->_method = other._method;
		this->_path = other._path;
	}
	return *this;
}

AHandler::~AHandler() {}