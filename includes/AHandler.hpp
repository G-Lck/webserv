#ifndef AHANDLER_HPP
#define AHANDLER_HPP

#include "WebServ.hpp"
#include "Client.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "HttpException.hpp"

class AHandler
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

		void	findServer();
		void	findLocation();
		void	checkRedirection();
		void	checkMethod();
		void	checkDotsPath();
		void	constructPath();

	public:
		AHandler();
		AHandler(Client client, ServerConfig virtual_server);
		AHandler(const AHandler& other);
		AHandler& operator=(const AHandler& other);
		~AHandler();
		void	run(); // should be virtual
};

#endif
