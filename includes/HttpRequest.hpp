#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include "HttpException.hpp"


class HttpRequest {

	private:

		std::string _method; // GET POST...
		std::string _path; // /home/images/...
		std::string _query_string;
		std::string _version; // HTTP//1.1
		std::map<std::string, std::string> _headers;
		std::string _body;

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

		void	printRequest() const;

};

#endif