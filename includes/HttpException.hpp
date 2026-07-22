#ifndef HTTPEXCEPTION_HPP
#define HTTPEXCEPTION_HPP

#include "WebServ.hpp"
#include "utils.hpp"

class HttpException: public std::exception
{

	private:
		int			_code;
		std::string	_message;

	public:
		HttpException(int code, const std::string& msg);
		virtual ~HttpException() throw() {}

		int	getCode() const;
		const char*	what() const throw();
		std::string	getResponseStr() const;
};

#endif
#ifndef HTTPEXCEPTION_HPP
#define HTTPEXCEPTION_HPP

#include <exception>
#include <string>

class HttpException: public std::exception
{

	private:
		int			_code;
		std::string	_message;

	public:
		HttpException(int code, const std::string& msg);
		virtual ~HttpException() throw() {}

		int	getCode() const;
		const char*	what() const throw();
};

#endif