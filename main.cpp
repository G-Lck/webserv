#include "./includes/WebServ.hpp"
#include "./includes/GlobalConfig.hpp"
#include "./includes/ParseConfig.hpp"
#include "./includes/EpollServer.hpp"
#include "./includes/PollServer.hpp"
#include "./includes/utils.hpp"

bool g_log = true;

int	main( int ac, char **av )
{
	if (ac < 2)
	{
		std::cout << "Wrong amount of parameters\nUsage: ./WebServ + [dir-to-config-file]. + \nOr Flag --test-config to test parsing\nOr Flag --no-log to deactivate the logs" << std::endl;
		return (-1);
	}
	signal(SIGCHLD, SIG_IGN);
	bool testMode = (ac >= 3 && std::string(av[2]) == "--test-config");
	if ((ac >= 3 && std::string(av[2]) == "--no-log"))
		g_log = false;

	GlobalConfig	Config;
	
	try
	{
		ParseConfig     Parse(av[1]);
		Parse.parse(Config);
		printConfig(Config, 1);
		writeLog(stringifyConfig(Config), 1);
		if (testMode)
			return (0);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		printConfig(Config, 0);
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

//- When send() returns EPIPE, kernel sends your process a SIGPIPE signal. Default behavior of SIGPIPE is to terminate the process
//- This can happen when a single client close their connection mid-write (could crash your entire server).
//- Here we mute this signal and we handle errors from send() ourselves, instead of killing the server.
// signal(SIGPIPE, SIG_IGN);