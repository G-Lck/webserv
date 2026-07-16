#include "../../includes/HttpResponse.hpp"

HttpResponse::HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse& other) { *this = other; }

HttpResponse& HttpResponse::operator=(const HttpResponse& other) {
	if (this != &other) {
		this->_status_code = other._status_code;
		this->_status_message = other._status_message;
		this->_headers = other._headers;
		this->_body = other._body;
		this->_connection = other._connection;
	}
	return *this;
}

HttpResponse::~HttpResponse() {}