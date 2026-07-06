#include "./includes/WebServ.hpp"
#include "./includes/GlobalConfig.hpp"
#include "./includes/ParseConfig.hpp"
#include "./includes/EpollServer.hpp"
#include "./includes/PollServer.hpp"
#include "./includes/Log.hpp"
#include "./includes/utils.hpp"

int	main( int ac, char **av )
{
	if (ac < 2)
	{
		std::cout << "Wrong amount of parameters\nExpected [dir-to-config-file]. \nOr Flag --test-config to test parsing" << std::endl;
		return (-1);
	}
	//- When send() returns EPIPE, kernel sends your process a SIGPIPE signal. Default behavior of SIGPIPE is to terminate the process
	//- This can happen when a single client close their connection mid-write (could crash your entire server).
	//- Here we mute this signal and we handle errors from send() ourselves, instead of killing the server.
	// signal(SIGPIPE, SIG_IGN);
	ParseConfig		Parse(av[1]);
	GlobalConfig	Config;
	logError("info1", 2);
	logError("info2", 2);
	logError("warning", 3);

	try
	{
		Parse.parse(Config);
		printConfig(Config, 1);
		logError(stringifyConfig(Config, 1), 1);
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

**5. Fatal errors inside run() loop (design decision, needs an answer)**
- `epoll_wait` returning a non-`EINTR` error — is that print-and-exit, or controlled shutdown first?
- Your destructor unwind already closes sockets/clients correctly via RAII, which is good — but confirm that's a deliberate design, not an accident. If it's deliberate: fine, just document it with a comment so it doesn't look accidental. If you want extra safety, you could log "shutting down due to fatal epoll error" before the throw so it's traceable in logs.
*/