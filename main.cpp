#include "./includes/WebServ.hpp"
#include "./includes/GlobalConfig.hpp"
#include "./includes/Server.hpp"
#include "./includes/ParseConfig.hpp"

int	main( int ac, char **av )
{
	if (ac != 2)
	{
		std::cout << "Wrong amount of parameters\nExpected [dir-to-config-file] or --test-config or --test-http" << std::endl;
		return (-1);
	}
	ParseConfig		Parse(av[1]);
	GlobalConfig	Config;
	Server 			Serv42;

	try
	{
		Parse.parse(Config);
		//+ CHECK PARSE FUNCTION MISSING:
		//	- check if root is not empty
		//	- check at least one server
		//	~ TODO: Two servers on exact same host:port with no server_name → conflict 
		//+ ON SUCCESS, PRINT PARSE FUNCTION
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
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
