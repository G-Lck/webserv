#ifndef HANDLERCGI_HPP
#define HANDLERCGI_HPP

#include "Handler.hpp"
#include "WebServ.hpp"
#include "ParseConfig.hpp"

class CgiHandler : public Handler
{
	private:
		pid_t		_pid;			// Child process id, for waitpid/kill.
		int			_stdinFd;		// Write end of the pipe (set to -1 when already closed)
		int			_stdoutFd;		// Read end of the pipe (set to -1 when already closed)
		bool		_finished;		// For disconnectCGI/timeout logic avoiding double-kill/double-reap.
		std::string	_writeBuffer;	// Body sent to the child
		size_t		_writeOffset;	// Body bytes alredy sent count
		std::string	_readBuffer;	// To append every time we read from stdout
		char**		_env;			// The CGI env variables array
		time_t		_startTime;		// For the timeout
		int			_exitStatus;	// Result of waitpid, to distinguish clean exit vs signal-killed (needed for 502 decision).
		std::string	_scriptPath;	// Resolved path to the CGI executable, needed at execve time.
		Client*		_client;		// To attach the eventual response to the right client once done.

		CgiHandler(CgiHandler const &other);
		CgiHandler& operator=(CgiHandler const &other);
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
		void	switchToRead();

		//+ Getters
		pid_t				getPid() const;
		int					getStdinFd() const;
		int					getStdoutFd() const;
		bool				isFinished() const;
		const std::string&	getWriteBuffer() const;
		size_t				getWriteOffset() const;
		const std::string&	getReadBuffer() const;
		char**				getEnv() const;
		time_t				getStartTime() const;
		int					getExitStatus() const;
		const std::string&	getScriptPath() const;
		Client*				getClient() const;

		//+ Monitoring
		void	setStartTime();
		bool	cgiTimeout();

		//+ Clean up
		void	closeStdinFd();
		void	closeStdoutFd();
};

std::ostream &operator<<(std::ostream &out, CgiHandler const &cgi);

#endif