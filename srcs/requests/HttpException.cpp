#include "../../includes/HttpException.hpp"

HttpException::HttpException(int code, const std::string& message): _code(code), _message(message) {}

int	HttpException::getCode() const
{
	return(this->_code);
}

const char*	HttpException::what() const throw()
{
	return _message.c_str();
}

std::string buildDefaultResponse(std::string msg)
{
	std::string response = "HTTP/1.1 " + msg + "\r\n";
	response += "Content-Length: 0\r\n";
	response += "Connection: close\r\n\r\n";

	return (response);
}

std::string HttpException::getResponseStr()
{
	writeLog(this->_message, STATUS_CODE);

	switch (this->_code)
	{
		case 400:
			return (buildDefaultResponse("400 Bad Request"));
		case 403:
			return (buildDefaultResponse("403 Forbidden"));
		case 404:
			return (buildDefaultResponse("404 Not Found"));
		case 405:
			return (buildDefaultResponse("405 Method Not Allowed"));
		case 413:
			return (buildDefaultResponse("413 Payload Too Large"));
		case 500:
			return (buildDefaultResponse("500 Internal Server Error"));
		case 501:
			return (buildDefaultResponse("501 Not Implemented"));
		case 505:
			return (buildDefaultResponse("505 HTTP Version Not Supported"));
		default:
			return (buildDefaultResponse("500 Internal Server Error"));
	}
}