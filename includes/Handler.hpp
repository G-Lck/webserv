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
	private:
		Client						_client;
		HttpRequest					_http_request;
		HttpResponse*				_http_response;
		std::vector<ServerConfig>	_virtual_servers;
		ServerConfig				_my_virtual_server;
		LocationConfig				_location;
		std::string					_method;
		std::string					_path;

		void	checkDotsPath();
		void	findServer();
		void	findLocation();
		void	checkRedirection();
		void	checkMethod();
		void	constructPath();

	public:
		Handler();
		Handler(Client client, const std::vector<ServerConfig> &virtual_servers);
		Handler(const Handler& other);
		Handler& operator=(const Handler& other);
		~Handler();
		void	run();
};

#endif
