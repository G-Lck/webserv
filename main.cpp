#include "./includes/WebServ.hpp"
#include "./includes/GlobalConfig.hpp"
#include "./includes/EpollServer.hpp"

int	main( void )
{

	#ifndef __linux__

	#endif

	GlobalConfig	Config;
	EpollServer 	Serv42;

	try
	{
		Serv42.configAServer(Config);
		Serv42.initAServer();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}
