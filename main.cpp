#include "./includes/WebServ.hpp"
#include "./includes/GlobalConfig.hpp"
#include "./includes/Server.hpp"
#include "./includes/ParseConfig.hpp"
#include "./includes/EpollServer.hpp"
#include "./includes/PollServer.hpp"

int	main( int ac, char **av )
int	main(void)
{
	if (ac < 2)
	{
		std::cout << "Wrong amount of parameters\nExpected [dir-to-config-file]. \nOr Flag --test-config to test parsing" << std::endl;
		return (-1);
	}
	ParseConfig		Parse(av[1]);
	GlobalConfig	Config;

	try
	{
		Parse.parse(Config);
		printConfig(Config, 1);
		//+ CHECK PARSE FUNCTION MISSING:
		//	- check if root is not empty
		//	- check at least one server
		//	~ TODO: Two servers on exact same host:port with no server_name → conflict 
		//+ ON SUCCESS, PRINT PARSE FUNCTION
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		printConfig(Config, 0);
		return (1);
	}

	if (ac < 3)
	{
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
	#ifdef __linux__
	EpollServer 	Serv42;
	#else
	PollServer 		Serv42;
	#endif

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
