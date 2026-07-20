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
		ServerConfig				_my_virtual_server;
		LocationConfig				_location;
		std::string					_method;
		std::string					_path;

		void	checkDotsPath();
		void	findServer(const std::vector<ServerConfig> &virtual_servers);
		void	findLocation();
		void	checkRedirection();
		void	checkMethod();
		void	constructPath();

	public:
		Handler();
		Handler(Client* client);
		Handler(const Handler& other);
		Handler& operator=(const Handler& other);
		~Handler();
		
		//+ ------ MAIN FUNCTIONALLITY ------
		void	initHandler(const std::vector<ServerConfig> &virtual_servers);
		bool	isCGI();
		void	executeStatic();

		//+ ------ GETTERS ------
		Client*					getClient() const;
		const HttpRequest&		getHttpRequest() const;
		const ServerConfig&		getVirtualServer() const;
		const LocationConfig&	getLocation() const;
		const std::string&		getMethod() const;
		const std::string&		getPath() const;
};

#endif
