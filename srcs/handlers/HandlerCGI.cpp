#include "../../includes/HandlerCGI.hpp"

// ---------- ORTHODOX ----------

CgiHandler::CgiHandler() : _pid(-1), _parentFdIn(-1), _parentFdOut(-1), _childFdOut(-1), _childFdIn(-1), _finished(false), _startTime(time(NULL)) {}

CgiHandler::~CgiHandler()
{
	this->closeAllFd();
	this->killCgi();
}

void	CgiHandler::killCgi()
{
    if (_pid == -1)
        return;
	
    // Ignore errors if the process already exited.
    kill(this->_pid, SIGKILL);
    
	this->_pid = -1;
}

void	CgiHandler::closeAllFd()
{
	close(this->_childFdIn);
	close(this->_childFdOut);
	close(this->_parentFdIn);
	close(this->_parentFdOut);
}

CgiHandler::CgiHandler(Handler const &handler) : Handler(handler)
{
	this->_client = handler.getClient();
	this->_scriptPath = handler.getPath();
	this->_parentFdIn = -1;
	this->_parentFdOut = -1;
	this->_finished = false;
	this->_pid = -1;
	this->_startTime = time(NULL);
	this->_exitStatus = 0;
	this->_env.push_back(NULL);
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
	return out;
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
	HttpRequest 				req = this->getRequestHandler();

	//+ Hard-coded ones
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env.push_back("SERVER_SOFTWARE=webserv/1.0");
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");

	//+ From HttpRequest
	env.push_back("REQUEST_METHOD=" + req.getMethod());
	env.push_back("QUERY_STRING=" + req.getQueryString());
	env.push_back("CONTENT_TYPE=" + req.getHeaders().find("Content-Type")->second);
	env.push_back("CONTENT_LENGTH=" + req.getHeaders().find("Content-Length")->second);
	env.push_back("SCRIPT_NAME=" + splitPath(0));
	env.push_back("PATH_INFO=" + splitPath(1));
	
	//+ ServerConfig matches
	env.push_back("SERVER_NAME=" + this->getServerName());
	env.push_back("SERVER_PORT=" + this->getClient()->getHostPort().second);

	//+ From socket/connection
	// REMOTE_ADDR ← client's sockaddr (from your Socket/connection object)
	env.push_back("REMOTE_ADDR=" + this->getClient()->getRemoteAddress());
	// REMOTE_HOST — skip, not doing reverse DNS
	// AUTH_TYPE, REMOTE_USER, REMOTE_IDENT — skip, no auth implemented

	//+ Populate
	std::vector<char*> envp;
	for (size_t i = 0; i < env.size(); i++)
		envp.push_back(const_cast<char*>(env[i].c_str()));
	envp.push_back(NULL);
}

std::string	CgiHandler::splitPath(int flag)
{
	std::string ext = this->_location.getCgiHandler().first;
	size_t pos = this->getPath().find(ext);
	if (pos != std::string::npos)
	{
		pos += ext.size();
		if (flag == 0)
			return (this->getPath().substr(0, pos));
		else
			return (this->getPath().substr(pos));
	}
	else
	{
		if (flag == 0)
			return (this->getPath());
		else
			return ("");
	}
}

void	CgiHandler::openPipe()
{
	try
	{
		validateCgi();
	}
	catch(const HttpException& e)
	{
		throw ;
	}
	setEnvVars();
	
    int fd_res[2], fd_req[2];

    //+ error checking for pipe
    if (pipe(fd_res) < 0 || pipe(fd_req) < 0)
	{
		std::stringstream err;
		err << "pipe err: " << strerror(errno);
		writeLog(err.str(), ERROR_INFO);
		close(fd_res[0]);
		close(fd_res[1]);
		close(fd_req[0]);
		close(fd_req[1]);
		throw HttpException(500, "Pipe error in CgiHandler::openPipe()");
	}
	this->_parentFdOut = fd_req[1];
	this->_parentFdIn = fd_res[0];
	this->_childFdIn = fd_req[0];
	this->_childFdOut = fd_res[1];

    //+ error checking for fcntl
    if (fcntl(fd_res[0], F_SETFL, O_NONBLOCK) < 0 || fcntl(fd_req[0], F_SETFL, O_NONBLOCK) < 0 ||
			fcntl(fd_res[1], F_SETFL, O_NONBLOCK) < 0 || fcntl(fd_req[1], F_SETFL, O_NONBLOCK) < 0)
	{
		std::stringstream err;
		err << "fnctl err: " << strerror(errno);
		writeLog(err.str(), ERROR_INFO);
		closeAllFd();
		throw HttpException(500, "fnctl error in CgiHandler::openPipe()");
	}

   	this->_pid = fork();
	if (this->_pid == -1)
	{
		std::stringstream err;
		err << "fork err: " << strerror(errno);
		writeLog(err.str(), ERROR_INFO);
		closeAllFd();
		throw HttpException(500, "fork error in CgiHandler::openPipe()");
	}

	if (this->_pid == 0)
	{
		// close the other end
		if (dup2(this->_childFdIn, STDIN_FILENO) == -1)
		{
			closeAllFd();
			exit(-1);
		}
		// close he other end
		if (dup2(this->_childFdOut, STDOUT_FILENO) == -1)
		{
			closeAllFd();
			exit(-1);
		}
		//~ CHECK IF WE NEED TO CLOSE ALL FD NOW OR NOT
		//EXECUTE THE CGI
		char *buff[10000];
		read(STDIN_FILENO, buff, 100);
		std::cout << "Buffer says: " << buff << std::endl;
		exit(1);
	}
	else
	{
		close(this->_childFdIn);
		close(this->_childFdOut);
	}
	// add to epoll the write fd and set to epoll out
	//startWriting();                                                                                                  
}

// ---------- REQUEST/RESPONSE PROCESING ----------


/*
* What the response needs
_status_code
_status_message
_headers
_body
_connection
*/

void	CgiHandler::continueReading()
{
	// refresh client and cgi time
    //     read available data from fd into handler's read buffer
    //     if read returns 0 (EOF):
    //         mark cgi output finished
    //         close stdout fd, remove from epoll
    //         build HTTP response from read buffer
    //         attach response to handler's _client
    //         re-arm client fd for EPOLLOUT
    //         cleanup: erase fd(s) from AServer's cgi map, delete handler

}

void	CgiHandler::switchToRead()
{
	// remove from multiplexer, change the int fd in the map of AServer, close both ends of that pipe 
	// add to multiplexer the read end and set to epoll in
}

void	CgiHandler::continueWriting()
{
	// refresh client and cgi time
	//     write next chunk of handler's write buffer to fd
    //     if all body written:
			// switchToRead();
}


// ---------- GETTERS ----------

pid_t	CgiHandler::getPid() const { return this->_pid; }

int	CgiHandler::getStdinFd() const { return this->_parentFdIn; }

int	CgiHandler::getStdoutFd() const { return this->_parentFdOut; }

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
