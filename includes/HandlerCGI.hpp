#ifndef HANDLERCGI_HPP
#define HANDLERCGI_HPP

#include "WebServ.hpp"
#include "ParseConfig.hpp"
#include "Handler.hpp"

class CgiHandler : public Handler
{
	private:
		pid_t						_pid;				// Child process id, for waitpid/kill.
		int							_parentFdIn;		// Write end of the pipe (set to -1 when already closed)
		int							_parentFdOut;		// Read end of the pipe (set to -1 when already closed)
		int							_childFdOut;
		int							_childFdIn;
		bool						_finishedWritting;
		bool						_finished;			// For disconnectCGI/timeout logic avoiding double-kill/double-reap.
		std::string					_writeBuffer;		// Body sent to the child
		size_t						_writeOffset;		// Body bytes alredy sent count
		std::string					_readBuffer;		// To append every time we read from stdout
		std::vector<char*>			_env;				// The CGI env variables array
		std::vector<std::string>	_envStrings;
		time_t						_startTime;			// For the timeout
		int							_exitStatus;		// Result of waitpid, to distinguish clean exit vs signal-killed (needed for 502 decision).
		std::string					_scriptPath;		// Resolved path to the CGI executable, needed at execve time.
		Client*						_client;			// To attach the eventual response to the right client once done.
		std::string					_response_headers;
		std::string					_response_body;

		CgiHandler& operator=(CgiHandler const &other);
		std::string	splitPath(int flag);
		CgiHandler(CgiHandler const &other);
	public:
		CgiHandler();
		~CgiHandler();
		CgiHandler(Handler const &handler);

		//+ Setup
		void	validateCgi();
		void	setEnvVars();
		
		//+ Execution
		void	openPipe();
		void	continueReading();
		void	continueWriting();
		void	closeParentFdOut();
		void	parseCgiResponse();
		bool	splitHeadersAndBody();
		bool	parseResponseHeaders();

		//+ Getters
		pid_t				getPid() const;
		int					getParentFdIn() const;
		int					getParentFdOut() const;
		bool				isFinished() const;
		bool				finishedWriting() const;
		const std::string&	getWriteBuffer() const;
		size_t				getWriteOffset() const;
		const std::string&	getReadBuffer() const;
		std::vector<char*>	getEnv() const;
		time_t				getStartTime() const;
		int					getExitStatus() const;
		const std::string&	getScriptPath() const;
		Client*				getClient() const;
		int					getStdinFd() const;
		int					getStdoutFd() const;

		//+ Monitoring
		void	updateCgiTime();
		bool	cgiTimeout();

		//+ Clean up
		void	closeAllFd();
		void	killCgi();

		//+ Testing
		void	setWriteBuffer(std::string str);
};

std::ostream &operator<<(std::ostream &out, CgiHandler const &cgi);

#endif