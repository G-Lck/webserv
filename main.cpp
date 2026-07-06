#include "./includes/WebServ.hpp"
#include "./includes/GlobalConfig.hpp"
#include "./includes/ParseConfig.hpp"
#include "./includes/EpollServer.hpp"
#include "./includes/PollServer.hpp"

int	main( int ac, char **av )
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
1. Resource ownership — where does a failure leave a dangling fd or pointer?
Walk through AServer.cpp:93-128 (addNewClient) step by step:

Line 98: accept() succeeds → you have a real fd.
Line 106: fcntl() fails → you print a message and return false.
Question for you: at that return false, what happens to fd_client? Trace it — is it closed anywhere? Is it added to _clients? What does the caller (run() in Epoll/PollServer) do when addNewClient returns false? If the answer is "nothing, it just moves on," you have an fd leak every time fcntl fails on a client socket. Rare in practice, but under fd-exhaustion conditions it's exactly the moment this would start happening — and now you've made it worse.

Same category, different function: AServer.cpp:21-43 configAServer. newSocket = new Socket on line 34, then newSocket->makeSocket(...) on line 35. makeSocket (in Socket.cpp) throws on any of getaddrinfo/socket/setsockopt/bind/listen/fcntl failing. Question: if it throws, does anything ever delete newSocket? Is it in _sockets yet at that point? Look at where addSocket is called relative to where the exception could be thrown.

2.
Here's the fix list, grouped by what's actually broken:

**1. epoll_ctl exception propagation (critical)**
- Single client's failed `epoll_ctl` currently propagates all the way to `main()` and kills the whole server.
- Fix: catch it at `addNewClient` level (or in `EpollServer` itself), log it, drop that one client, `return false` — don't rethrow past the per-connection boundary. Reserve the throw-to-main path for things like epoll instance creation failing at startup, not per-client registration.

**2. recv/send error handling (critical, correctness bug)**
- `<= 0` is being treated as unconditional disconnect. `-1` can also mean `EAGAIN`/`EWOULDBLOCK`/`EINTR` — transient, not a disconnect.
- Fix: check `errno` when return is `-1`. Only treat as real disconnect if `recv() == 0` (peer closed) or a genuine error (not `EAGAIN`/`EWOULDBLOCK`/`EINTR`). On `EAGAIN`/`EINTR`, just return and try again later — don't close the client.

**3. std::cout vs std::cerr consistency**
- Runtime failure logs at those two lines go to `stdout`, everything else error-related goes to `stderr`.
- Fix: all error/failure logging → `std::cerr`. Reserve `stdout` for normal operational output if you have any. Otherwise your errors vanish into the wrong stream once someone redirects logs.

**4. main() exit codes (critical for eval/scripting)**
- Second try/catch (config/init) has no `return` after catching, and no `return` at end of `main()` at all.
- Fix: `return(1)` after that catch block, same as the first one. And add explicit `return(0)` at the true success end of `main()`. Right now a caller can't distinguish "started fine" from "crashed after printing an error" — that's a real bug, not style.

**5. Fatal errors inside run() loop (design decision, needs an answer)**
- `epoll_wait` returning a non-`EINTR` error — is that print-and-exit, or controlled shutdown first?
- Your destructor unwind already closes sockets/clients correctly via RAII, which is good — but confirm that's a deliberate design, not an accident. If it's deliberate: fine, just document it with a comment so it doesn't look accidental. If you want extra safety, you could log "shutting down due to fatal epoll error" before the throw so it's traceable in logs.

Priority order: **#2 first** (silently killing healthy clients is the worst kind of bug — you won't see it until eval day under load), then **#1**, then **#4**, then **#3**, then **#5** as a design confirmation.

4. Buffer growth — you already flagged this yourself
Your own comment block at the bottom of AServer.cpp:188-206 already lists this, and I confirmed it: _client_max_body_size is parsed and stored per-config (ServerConfig.cpp, LocationConfig.cpp) but grep shows it's never read anywhere in srcs/server/. readRequest (AServer.cpp:130-144) appends to the read buffer with no cap. Question: what stops a client from sending gigabytes of data with no \r\n\r\n and growing that buffer indefinitely? Since you already wrote this down yourself, I'd just ask: is this next on your list, or did it get lost under other priorities?

5. Structural, not urgent yet
EpollServer.cpp:118-140, the #else branch — is this genuinely a fallback you're maintaining, or dead code copy-pasted for compilation on non-Linux? If you're only evaluating/running on Linux, ask yourself whether keeping a whole fake implementation around is helping you or just adding a second place bugs can hide.
PollServer::run() (PollServer.cpp:39-91) checks POLLERR|POLLHUP|POLLNVAL and disconnects; EpollServer::run() (EpollServer.cpp:50-79) does not check EPOLLERR/EPOLLHUP at all — your own comment #5 already caught this. Question: why did one implementation get this check and the other didn't? Since both derive from the same AServer contract, should behavior on a dead/error'd fd be identical between them?
Take these one at a time — I'd start with the fcntl fd-leak and the recv/send errno check, since those are the two that can bite you in a working demo (an evaluator triggering EINTR or a client dropping mid-fcntl failure), not just in theory. */