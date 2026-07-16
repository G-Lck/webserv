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

int	HttpResponse::getStatusCode() const
{
	return this->_status_code;
}

const std::string&	HttpResponse::getStatusMessage() const
{
	return this->_status_message;
}

std::map<std::string, std::string>	HttpResponse::getHeaders() const
{
	return this->_headers;
}

const std::string&	HttpResponse::getBody() const
{
	return this->_body;
}

bool	HttpResponse::getConnection() const
{
	return this->_connection;
}

std::ostream &operator<<(std::ostream &out, HttpResponse const &req )
{
	out << req.getStatusCode() << " " << req.getStatusMessage() << " ";
	
	std::map<std::string, std::string> headers_list = req.getHeaders();
	std::map<std::string, std::string>::const_iterator it = headers_list.begin();
	while (it != headers_list.end())
	{
		out << it->first << ":" << it->second << " ";
		it++;
	}
	out << req.getBody();
	return out;
}