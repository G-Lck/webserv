#include "../../includes/HttpException.hpp"

/// @brief Throw exeption
/// @param code 4xx-5xx
/// @param message A message to store in the class, used for logging, not for creating reponse
HttpException::HttpException(int code, const std::string& message): _code(code), _message(message) {}

int	HttpException::getCode() const
{
	return(this->_code);
}

const char*	HttpException::what() const throw()
{
	return _message.c_str();
}