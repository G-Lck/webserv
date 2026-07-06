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