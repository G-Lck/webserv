#include "./includes/WebServ.hpp"
#include "./includes/GlobalConfig.hpp"
#include "./includes/Server.hpp"

int	main( void )
{
	GlobalConfig	Config;
	Server 			Serv42;

	try
	{
		Serv42.configServer(Config);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	Serv42.epollInit();
	Serv42.epollAddSockets();
	Serv42.initServer();
}
