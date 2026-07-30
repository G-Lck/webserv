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


void HttpResponse::setStatusCode(int code)
{
    this->_status_code = code;
}

void HttpResponse::setStatusMessage(const std::string &message)
{
    this->_status_message = message;
}

void HttpResponse::setHeaders(const std::map<std::string, std::string> &headers)
{
    this->_headers = headers;
}

void HttpResponse::setHeader(const std::string &key, const std::string &value)
{
    this->_headers[key] = value;
}

void HttpResponse::removeHeader(const std::string &key)
{
    this->_headers.erase(key);
}

void HttpResponse::clearHeaders()
{
    this->_headers.clear();
}

void HttpResponse::setBody(const std::string &body)
{
    this->_body = body;
}

void HttpResponse::setConnection(bool keepAlive)
{
    this->_connection = keepAlive;
}

std::string	HttpResponse::buildResponseStr() const
{
	std::ostringstream oss;
	oss << "HTTP/1.1 " << this->_status_code << " " << this->_status_message << "\r\n";

	std::map<std::string, std::string> headers = this->_headers; // local copy
	std::ostringstream len;
	len << this->_body.size();
	std::map<std::string, std::string>::iterator it = headers.find("Content-Length");
	if (it != headers.end())
		it->second = len.str();
	else
		headers.insert(std::make_pair("Content-Length", len.str()));

	it = headers.begin();
	for (; it != headers.end(); ++it)
		oss << it->first << ": " << it->second << "\r\n";

	oss << "\r\n" << this->_body;
	return oss.str();
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