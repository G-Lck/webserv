#include "./includes/WebServ.hpp"
#include "./includes/GlobalConfig.hpp"
#include "./includes/ParseConfig.hpp"
#include "./includes/EpollServer.hpp"
#include "./includes/PollServer.hpp"
#include "./includes/utils.hpp"

int	main( int ac, char **av )
{
	if (ac < 2)
	{
		std::cout << "Wrong amount of parameters\nExpected [dir-to-config-file]. \nOr Flag --test-config to test parsing" << std::endl;
		return (-1);
	}
	bool testMode = (ac >= 3 && std::string(av[2]) == "--test-config");
	//- When send() returns EPIPE, kernel sends your process a SIGPIPE signal. Default behavior of SIGPIPE is to terminate the process
	//- This can happen when a single client close their connection mid-write (could crash your entire server).
	//- Here we mute this signal and we handle errors from send() ourselves, instead of killing the server.
	// signal(SIGPIPE, SIG_IGN);
	ParseConfig		Parse(av[1]);
	GlobalConfig	Config;

	try
	{
		Parse.parse(Config);
		printConfig(Config, 1);
		writeLog(stringifyConfig(Config, 1), 1);
		//+ CHECK PARSE FUNCTION MISSING:
		//	- check if root is not empty
		//	- check at least one server
		//	~ TODO: Two servers on exact same host:port with no server_name → conflict 
		//+ ON SUCCESS, PRINT PARSE FUNCTION
		if (testMode)
		{
			std::cout << "Parser passed." << std::endl;
			return (0);
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		printConfig(Config, 0);
		if (testMode)
		{
			std::cout << "Parser Failed." << std::endl;
		}
		return (1);
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

/*
 dont use waitpid for the gci case
 use non block
 add timeout in cgi
 
*/

// Questions for Garance:
// What h appens if the request never finishes with /n/r/n/r
// Where do we check for thgat? I cannot find it