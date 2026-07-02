#include "../../includes/HttpRequest.hpp"

HttpRequest::HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest& other) { *this = other; }

HttpRequest& HttpRequest::operator=(const HttpRequest& other) {
	if (this != &other)
	{
		this->_method = other._method;
		this->_path = other._path;
		this->_version = other._version;
		this->_headers = other._headers;
		this->_body = other._body;
	}
	return *this;
}

HttpRequest::~HttpRequest() {}

void	HttpRequest::parseRequestLine(const std::string& line)
{
	std::istringstream ss(line);
	std::string method, path, version, extra;
	ss >> method;
	ss >> path;
	ss >> version;

	if (ss >> extra)
		throw HttpException(400, "Bad Request");

	//we will have to throw exception if one of them is wrong;
}

void	HttpRequest::parseHeaders(const std::string& raw)
{
	(void) 	raw;
	//I think we will also have to check a lot and return execption.
}

bool	HttpRequest::parseBody(const std::string& raw)
{
	this->_body = raw;
	//clearly more than that to do
	return (true);
}

bool HttpRequest::parse(const std::string& raw) {

    size_t header_end = raw.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;

    parseRequestLine(raw.substr(0, header_end));
    parseHeaders(raw.substr(0, header_end));


    if (this->_headers.count("Content-Length")) {
        size_t body_len   = atoi(this->_headers["Content-Length"].c_str());
        size_t body_start = header_end + 4;
        size_t received   = raw.size() - body_start;

        if (received < body_len)
            return false;

        this->_body = raw.substr(body_start, body_len);
    }

    return true;
}