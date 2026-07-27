#include "../../includes/HandlerCGI.hpp"

// ---------- ORTHODOX ----------

/*
CgiHandler::CgiHandler() : _pid(-1), _stdinFd(-1), _stdoutFd(-1), _finished(false), _startTime(time(NULL)) { }

CgiHandler::~CgiHandler()
{
	// kill pid???
	this->closeStdinFd();
	this->closeStdoutFd();
}

void	CgiHandler::closeStdinFd()
{
	if (this->_stdinFd != -1)
		close(this->_stdinFd);
	this->_stdinFd = -1;
}

void	CgiHandler::closeStdoutFd()
{
	if (this->_stdoutFd != -1)
		close(this->_stdoutFd);
	this->_stdoutFd = -1;
}


CgiHandler::CgiHandler(Handler const &handler) : Handler(handler)
{
	this->_client = handler.getClient();
	this->_scriptPath = handler.getPath();
	this->_stdinFd = -1;
	this->_stdoutFd = -1;
	this->_finished = false;
	this->_pid = -1;
	this->_startTime = time(NULL);
	this->_exitStatus = 0;
	this->_env[0] = NULL;
	this->_writeOffset = 0;
}


// ---------- OVERLOAD ----------

std::ostream &operator<<(std::ostream &out, CgiHandler const &cgi)
{
	time_t start = cgi.getStartTime();
	char *t = ctime(&start);
	t[strlen(t) - 1] = '\0'; //+ remove the new line

	out << "Cgi Handler: pid[" << cgi.getPid() << "] - On Client [" << cgi.getClient()->getFd() << "] - ";
	out << "script-path: " << cgi.getScriptPath() << " Time of start: " << t;
}

// ---------- SETUP AND INIT ----------


/// @brief	Fucntion to validate the request values before starting with the cgi
///			checks: isValidFile(), validMethod(), validContentLength()
void	CgiHandler::validateCgi()
{
	if (!isValidFile(this->_path, X_OK)) //+ Check if valid file
		throw HttpException(404, "Cgi - Not Found");
	if (!validMethod())	//+ Check if allowed methods in location matches with request method
		throw HttpException(405, "Method Not Allowed");
	if (!validContentLength())
		throw HttpException(400, "Bad Request (Content-Length)");
}

void	CgiHandler::setEnvVars()
{
	std::vector<std::string>	env;
	HttpRequest 				req = this->getClient()->getRequest();

	//+ Hard-coded ones
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env.push_back("SERVER_SOFTWARE=webserv/1.0");
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");

	//+ From HttpRequest
	env.push_back("REQUEST_METHOD=" + req.getMethod());
	env.push_back("QUERY_STRING=" + req.getQueryString());
	env.push_back("CONTENT_TYPE=" + req.getHeaders().find("Content-Type")->second);
	env.push_back("CONTENT_LENGTH=" + req.getHeaders().find("Content-Length")->second);
	// SCRIPT_NAME / PATH_INFO ← derived by splitting getPath() against the CGI location's 
	//		extension match (not stored on HttpRequest as-is — you compute it in the CGI handler)
	
	//+ ServerConfig matches
	env.push_back("SERVER_NAME=" + this->getServerName());
	env.push_back("SERVER_PORT=" + this->getClient()->getHostPort().second);

	//+ From socket/connection
	// REMOTE_ADDR ← client's sockaddr (from your Socket/connection object)
	env.push_back("REMOTE_ADDR=" + this->getClient()->getRemoteAddress());
	// REMOTE_HOST — skip, not doing reverse DNS
	// AUTH_TYPE, REMOTE_USER, REMOTE_IDENT — skip, no auth implemented

	std::vector<char*> envp;
	for (size_t i = 0; i < env.size(); i++)
		envp.push_back(const_cast<char*>(env[i].c_str()));
	envp.push_back(NULL);
}



void	CgiHandler::openPipe()
{

}

// ---------- REQUEST/RESPONSE PROCESING ----------



* What the response needs
_status_code
_status_message
_headers
_body
_connection


void	CgiHandler::continueReading()
{
    //     read available data from fd into handler's read buffer
    //     if read returns 0 (EOF):
    //         mark cgi output finished
    //         close stdout fd, remove from epoll
    //         waitpid on handler's pid (non-blocking check or already reaped by monitorCgi)
    //         build HTTP response from read buffer
    //         attach response to handler's _client
    //         re-arm client fd for EPOLLOUT
    //         cleanup: erase fd(s) from AServer's cgi map, delete handler

}

void	CgiHandler::switchToRead()
{
	
}

void	CgiHandler::continueWriting()
{
	//     write next chunk of handler's write buffer to fd
    //     if all body written:
    //         close stdin fd, remove from epoll, mark handler side done
}


// ---------- GETTERS ----------

pid_t	CgiHandler::getPid() const { return this->_pid; }

int	CgiHandler::getStdinFd() const { return this->_stdinFd; }

int	CgiHandler::getStdoutFd() const { return this->_stdoutFd; }

bool	CgiHandler::isFinished() const { return this->_finished; }

const std::string&	CgiHandler::getWriteBuffer() const { return this->_writeBuffer; }

size_t	CgiHandler::getWriteOffset() const { return this->_writeOffset; }

const std::string&	CgiHandler::getReadBuffer() const { return this->_readBuffer; }

std::vector<char*>	CgiHandler::getEnv() const { return this->_env; }

time_t	CgiHandler::getStartTime() const { return this->_startTime; }

int	CgiHandler::getExitStatus() const { return this->_exitStatus; }

const std::string&	CgiHandler::getScriptPath() const { return this->_scriptPath; }

Client*	CgiHandler::getClient() const { return this->_client; }


// ---------- MONITORING ----------


void	CgiHandler::setStartTime() { time(&(this->_startTime)); }

bool	CgiHandler::cgiTimeout()
{
	time_t	now;
	time(&now);

	if (difftime(now, this->_startTime) > CGI_TIME_LIMIT)
		return (true);
	return(false);
}

*/