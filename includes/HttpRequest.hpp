#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "WebServ.hpp"
#include "HttpException.hpp"

class Client;

class HttpRequest {

	private:

		std::string							_method; // GET POST...
		std::string							_path; // /home/images/...
		std::string							_query_string;
		std::string							_version; // HTTP//1.1
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		size_t								_consumed_bytes;
		size_t								_global_max_body;

		void	parseRequestLine(const std::string& line);
		void	parseHeaders(const std::string& raw);
		bool	parseBody(const std::string& raw);
		bool	parseChunkedBody(const std::string& raw);
	public:
		HttpRequest();
		HttpRequest(const HttpRequest& other);
		HttpRequest& operator=(const HttpRequest& other);
		~HttpRequest();
		bool parse(const std::string& raw);

		// Getters
		size_t								getConsumedBytes() const;
		std::map<std::string, std::string>	getHeaders() const;
		const std::string&					getMethod() const;
		const std::string&					getPath() const;
		const std::string&					getQueryString() const;
		const std::string&					getVersion() const;
		const std::string&					getBody() const;
	
		void								printRequest() const;
};

std::ostream &operator<<(std::ostream &out, HttpRequest const &req );

#endif