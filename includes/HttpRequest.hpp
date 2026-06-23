#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>

class HttpRequest {

	private:

		std::string method; // GET POST...
		std::string path; // /home/images/...
		std::string version; // HTTP//1.1
		std::map<std::string, std::string> headers;
		std::string body;

		void parseRequestLine(const std::string& line);
		void parseHeaders(const std::string& raw);
		bool parseBody(const std::string& raw);

	public:

		HttpRequest();
		HttpRequesrt(const HttpRequest& other);
		HttpRequesrt& operator=(const HttpRequesrt& other);
		~HttpRequesrt();
		bool parse(const std::string& raw);
};

#endif