#ifndef Handler_HPP
#define Handler_HPP

#include "WebServ.hpp"
#include "Client.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "HttpException.hpp"

class Handler
{
	protected:
		Client*						_client;
		HttpRequest					_http_request;
		HttpResponse				_http_response;
		ServerConfig				_my_virtual_server;
		LocationConfig				_location;
		std::string					_method;
		std::string					_path;
		std::string					_server_name;
		std::string					_port;
		std::map<std::string, std::string> _response_headers;

		//+ ------ FIRST PARSING ------
		void	checkDotsPath();
		void	findServer(const std::vector<ServerConfig> &virtual_servers);
		void	findLocation();
		void	checkRedirection();
		void	checkMethod();
		void	constructPath();

		//+ ------ SECOND PARSING ------
		void	validateStatic();
		bool	validMethod();
		bool	validContentLength();

	public:
		Handler();
		Handler(Client* client);
		Handler(const Handler& other);
		Handler& operator=(const Handler& other);
		~Handler();

		//+ ------ MAIN FUNCTIONALLITY ------
		void	initAndParseHandler(const std::vector<ServerConfig> &virtual_servers);
		bool	isCGI() const;

		//+ ------ GETTERS ------
		Client*					getClient() const;
		const HttpRequest&		getRequestHandler() const;
		const HttpResponse&		getResponseHandler() const;
		const ServerConfig&		getVirtualServer() const;
		const LocationConfig&	getLocation() const;
		const std::string&		getMethod() const;
		const std::string&		getPath() const;
		const std::string&		getServerName() const;
		const std::map<std::string, std::string>&	getResponseHeaders() const;
		void					validateBodySize() const;
		void	setResponseHeader(const std::string &key, const std::string &value);

		bool					hasErrorPage(int code) const;
		std::string				CreateErrorPageContent(int code) const;
};

#endif
