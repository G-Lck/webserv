#include "../../includes/HttpRequest.hpp"
#include "../../includes/WebServ.hpp"

HttpRequest::HttpRequest(): _consumed_bytes(0) {}

HttpRequest::HttpRequest(const HttpRequest& other) { *this = other; }

HttpRequest& HttpRequest::operator=(const HttpRequest& other) {
	if (this != &other)
	{
		this->_method = other._method;
		this->_path = other._path;
		this->_query_string = other._query_string;
		this->_version = other._version;
		this->_headers = other._headers;
		this->_body = other._body;
		this->_consumed_bytes = other._consumed_bytes;
	}
	return *this;
}

HttpRequest::~HttpRequest() {}

void	HttpRequest::parseRequestLine(const std::string& line)
{
	std::istringstream	ss(line);
	std::string			method, path, version, extra;
	ss >> method;
	ss >> path;
	ss >> version;

	if (ss >> extra)
		throw HttpException(400, "Bad Request");

	if (!(method == "GET" || method == "POST" || method == "DELETE")) // faire un test si il y a un truc apres post
		throw HttpException(400, "Bad Request");
	this->_method = method;

	if (path.empty() || path[0] != '/')
		throw HttpException(400, "Bad Request");
	size_t q = path.find('?');
	if (q != std::string::npos) {
		this->_path         = path.substr(0, q);
		this->_query_string = path.substr(q + 1);
	} else {
		this->_path = path;
	}

	if (version != "HTTP/1.0" && version != "HTTP/1.1")
		throw HttpException(505, "HTTP Version Not Supported");
	this->_version = version;
}

void	HttpRequest::parseHeaders(const std::string& raw)
{
	std::istringstream	ss(raw);
	std::string			line;

	while (std::getline(ss, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (line.empty()) continue;

		size_t	sep = line.find(": ");
		if (sep == std::string::npos)
			throw HttpException(400, "Bad Request");

		std::string	key   = line.substr(0, sep);
		std::string	value = line.substr(sep + 2);
		this->_headers[key] = value;
	}
	if (this->_version == "HTTP/1.1" && !this->_headers.count("Host"))
		throw HttpException(400, "Bad Request");
}

bool HttpRequest::parseBody(const std::string& body_raw)
{
	size_t	body_len = atoi(_headers["Content-Length"].c_str());
	
	if (body_len > MAX_BODY) // should be something form config file
		throw HttpException(413, "Content Too Large");
	
	if (body_raw.size() < body_len)
		return false;
	this-> _body = body_raw.substr(0, body_len);
	this->_consumed_bytes += body_len;
	return true;
}

bool HttpRequest::parseChunkedBody(const std::string& raw) // we should check the max size
{
    std::string result;
    size_t pos = 0;

    while (pos < raw.size())
	{
        size_t chunk_end = raw.find("\r\n", pos);
        if (chunk_end == std::string::npos)
            return false;

        std::string size_str = raw.substr(pos, chunk_end - pos);
        size_t chunk_size = strtol(size_str.c_str(), NULL, 16);

        if (chunk_size == 0)
		{
            this->_consumed_bytes += chunk_end + 4; // "0\r\n\r\n"
            this->_body = result;
            return true;
        }
        pos = chunk_end + 2;
        if (pos + chunk_size > raw.size())
            return false;

        result += raw.substr(pos, chunk_size);
        pos += chunk_size + 2;
    }
    return false;
}

bool HttpRequest::parse(const std::string& raw)
{
	std::cout << "http request parser begin" << std::endl;
	this->_consumed_bytes = 0;

	size_t	header_end = raw.find("\r\n\r\n");

	if (header_end == std::string::npos)
		return false;

	this->_consumed_bytes = header_end + 4;

	size_t	first_line_end = raw.find("\r\n");
	parseRequestLine(raw.substr(0, first_line_end));
	parseHeaders(raw.substr(first_line_end + 2, header_end - first_line_end - 2));

	if (_headers.count("Transfer-Encoding") && _headers["Transfer-Encoding"] == "chunked")
		return parseChunkedBody(raw.substr(header_end + 4));
	else if (_headers.count("Content-Length"))
		return parseBody(raw.substr(header_end + 4));

	std::cout << "http request parser over" << std::endl;
	return true;
}

int	HttpRequest::getConsumedBytes() const
{
	return this->_consumed_bytes;
}

void	HttpRequest::printRequest() const
{
	std::cout << "method: " << _method << std::endl;
	std::cout << "path: " << _path << std::endl;
	std::cout << "query string: " << _query_string << std::endl;
	std::cout << "version: " << _version << std::endl;
	std::cout << "Headers: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << "Body" << std::endl << _body << std::endl;


}