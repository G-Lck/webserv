#include "../../includes/HttpRequest.hpp"

HttpRequest::HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest& other) { *this = other; }

HttpRequest& HttpRequest::operator=(const HttpRequest& other) {
	if (this != &other)
	{
		this->method = other.method;
		this->path = other.path;
		this->version = other.version;
		this->headers = other.headers;
		this->body = other.body;
	}
	return *this;
}

HttpRequest::~HttpRequest() {}

void	HttpRequest::parseRequestLine(const std::string& line)
{
	size_t	len;
	
	len = line.find(line, ' ');
	this->method = line.substr(0, len);
	line.erase(line, len);

	len = line.find(line, ' ');
	this->path = line.substr(0, len);
	line.erase(line, len);

	len = line.find(line, ' ');
	this->version = line.substr(0, len);
	line.erase(line, len);

	//we will have to throw exception if one of them is wrong;
}

void	HttpRequest::parseHeaders(const std::string& raw)
{
	size_t	len;

	while (1)
	{
		len = line.find(line, ' ');
		this->headers.insert(line.substr(0, len));
		line.erase(line, len);
		//break at some point
	}

	//I think we will also have to check a lot and return execption.
}

bool	HttpRequest::parseBody(const std::string& raw)
{
	this->body = raw;
	//clearly more than that to do
	return (true)
}

bool HttpRequest::parse(const std::string& raw) {

    size_t header_end = raw.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;

    parseRequestLine(raw.substr(0, header_end));
    parseHeaders(raw.substr(0, header_end));


    if (headers.count("Content-Length")) {
        size_t body_len   = atoi(headers["Content-Length"].c_str());
        size_t body_start = header_end + 4;
        size_t received   = raw.size() - body_start;

        if (received < body_len)
            return false;

        body = raw.substr(body_start, body_len);
    }

    return true;
}