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

		int									getStatusCode() const;
		const std::string&					getStatusMessage() const;
		std::map<std::string, std::string>	getHeaders() const;
		const std::string&					getBody() const;
		bool								getConnection() const;

		void								setStatusCode(int code);
		void								setStatusMessage(const std::string &message);
		void								setHeaders(const std::map<std::string, std::string> &headers);
		void								setHeader(const std::string &key, const std::string &value);
		void								removeHeader(const std::string &key);
		void								clearHeaders();
		void								setBody(const std::string &body);
		void								setConnection(bool keepAlive);

		std::string							buildResponseStr() const;
};

std::ostream &operator<<(std::ostream &out, HttpResponse const &res);

#endif