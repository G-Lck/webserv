#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "WebServ.hpp"

class HttpResponse
{
	private:
		int									_status_code;
		std::string							_status_message;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		bool								_connection;
	public:
		HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse& operator=(const HttpResponse& other);
		~HttpResponse();
};

#endif