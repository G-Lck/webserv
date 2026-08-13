#include "../../includes/HandlerCGI.hpp"

// ---------- ORTHODOX ----------

CgiHandler::CgiHandler() : _pid(-1), _parentFdIn(-1), _parentFdOut(-1), _childFdOut(-1), _childFdIn(-1), _finishedWritting(false), _finished(false), _startTime(time(NULL)) {}

CgiHandler::~CgiHandler()
{
	std::stringstream ss;
	ss << *this << "Terminating child process";
	writeLog(ss.str(), SERVER_EVENTS);
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
}

static std::string	headerToCgiFormat(const std::string &header_name, const std::string &header_value)
{
	std::string	ret = "HTTP_";

	for (size_t i = 0; i < header_name.size(); ++i)
	{
        char c = header_name[i];
		if (c == '-')
			ret += '_';
		else
			ret += std::toupper(static_cast<unsigned char>(c));
	}
	return (ret.append("=").append(header_value));
}

void	CgiHandler::setEnvVars()
{
	HttpRequest 										req = this->getRequestHandler();
	std::map<std::string, std::string>					headers = req.getHeaders();
	std::map<std::string, std::string>::const_iterator	h_it;	

	//+ Hard-coded ones
	this->_envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");
	this->_envStrings.push_back("SERVER_SOFTWARE=webserv/1.0");
	this->_envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");

	//+ From HttpRequest
	this->_envStrings.push_back("REQUEST_METHOD=" + req.getMethod());
	this->_envStrings.push_back("QUERY_STRING=" + req.getQueryString());

	h_it = headers.find("content-type");
	if (h_it != headers.end())
		this->_envStrings.push_back("CONTENT_TYPE=" + h_it->second);
	if (req.getMethod() == "POST")
	{
		std::ostringstream oss;
		oss << req.getBody().size();
		this->_envStrings.push_back("CONTENT_LENGTH=" + oss.str());
	}
	this->_envStrings.push_back("SCRIPT_NAME=" + splitPath(0));
	this->_envStrings.push_back("PATH_INFO=" + splitPath(1));
	if (!splitPath(1).empty())
    	this->_envStrings.push_back("PATH_TRANSLATED=." + this->getVirtualServer().getRoot() + splitPath(1));
	
	//+ ServerConfig matches
	this->_envStrings.push_back("SERVER_NAME=" + this->getServerName());
	this->_envStrings.push_back("SERVER_PORT=" + this->getClient()->getHostPort().second);

	//+ From socket/connection
	// REMOTE_ADDR ← client's sockaddr (from your Socket/connection object)
	this->_envStrings.push_back("REMOTE_ADDR=" + this->getClient()->getRemoteAddress());
	// REMOTE_HOST — skip, not doing reverse DNS
	// AUTH_TYPE, REMOTE_USER, REMOTE_IDENT — skip, no auth implemented

	//+ Headers
	for (h_it = headers.begin(); h_it != headers.end(); h_it++)
	{
		if (h_it->first == "content-type" || h_it->first == "content-length" || h_it->first == "connection")
			continue ;
		this->_envStrings.push_back(headerToCgiFormat(h_it->first, h_it->second));
	}

	//+ Populate
	_env.clear();
	for (size_t i = 0; i < _envStrings.size(); ++i)
		_env.push_back(const_cast<char*>(_envStrings[i].c_str()));
	_env.push_back(NULL);
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
	if (fcntl(fd_res[0], F_SETFL, O_NONBLOCK) < 0 || fcntl(fd_req[1], F_SETFL, O_NONBLOCK) < 0)
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
		if (dup2(this->_childFdIn, STDIN_FILENO) == -1)
		{
			closeAllFd();
			exit(-1);
		}
		if (dup2(this->_childFdOut, STDOUT_FILENO) == -1)
		{
			closeAllFd();
			exit(-1);
		}
		closeAllFd();

		// build argv
		char *argv[] = {
			const_cast<char*>("/usr/bin/python3"),
			const_cast<char*>(_scriptPath.c_str()),
			NULL
		};

		// exec
		execve(argv[0], argv, &this->_env[0]);
		// what with this error
		_exit(1);
	}
	else
	{
		std::stringstream log;
		log << "Child created: " << this->_pid << " ";
		writeLog(log.str(), SERVER_EVENTS);
		close(this->_childFdIn);
		close(this->_childFdOut);
	}
	updateCgiTime();
	// Here we go back to HandleCompleteRequest
	// we add to epoll and to the map<fd, cgiHandler>
	setWriteBuffer(this->getRequestHandler().getBody());
}

// ---------- REQUEST/RESPONSE PROCESING ----------


bool	CgiHandler::continueReading()
{
	char buffer[4096];

	//+ read (some) data from fd into handler's read buffer
	ssize_t n = read(_parentFdIn, buffer, sizeof(buffer));
	if (n < 0)
	{
		//+ man read(): EAGAIN or EWOULDBLOCK: The socket is marked nonblocking and the receive operation
		//+ would block, or a receive timeout had been set and the
		//+ timeout expired before data was received.
		//+ EINTR  The receive was interrupted by delivery of a signal before any data was available
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		{
			std::stringstream ss;
			ss << "read() error -> " << strerror(errno);
			writeLog(ss.str(), ERROR_WARNING);
			return (true);
		}
		//+ In any other case we log error
		std::stringstream ss;
		ss << "read() error -> " << strerror(errno);
		writeLog(ss.str(), ERROR_INFO);
		return (false);
	}
	
	if (n > 0)
	{		
		_readBuffer.append(buffer, n);
	}
	//+ if read returns 0 (EOF)
	else if (n == 0)
	{
		this->_finished = true;
	}
	//+ Update Time
	this->updateCgiTime();
	return (true);
}

bool	CgiHandler::continueWriting()
{
	//+ write next chunk of handler's write buffer to fd
	ssize_t n = write(_parentFdOut, _writeBuffer.data() + _writeOffset, _writeBuffer.size() - _writeOffset);
	if (n < 0)
	{
		//+ man write(): EAGAIN or EWOULDBLOCK: The socket is marked nonblocking and the receive operation
		//+ would block, or a receive timeout had been set and the
		//+ timeout expired before data was received.
		//+ EINTR  The receive was interrupted by delivery of a signal before any data was available
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		{
			std::stringstream ss;
			ss << "write() error -> " << strerror(errno);
			writeLog(ss.str(), ERROR_WARNING);
			return (true);
		}
		//+ In any other case we log error
		std::stringstream ss;
		ss << "write() error -> " << strerror(errno);
		writeLog(ss.str(), ERROR_INFO);
		return (false);
	}

	if (n > 0)
		_writeOffset += n;
	//+ Update Time
	this->updateCgiTime();

	//+ If all body written
	if (_writeOffset == _writeBuffer.size())
	{
		this->_finishedWritting = true;
	}
	return (true);
}

// ---------- CGI RESPONSE PARSING ----------

void CgiHandler::parseCgiResponse()
{
	this->_http_response.clearHeaders();

	//+ Check if valid format of end of response
	//+ Parse Headers
	if (!splitHeadersAndBody() || !parseResponseHeaders())
	{
		this->_http_response.setStatusCode(502);
		this->_http_response.setStatusMessage("Bad Gateway");
		return ;
	}

	//+ Body is copied exactly
	this->_http_response.setBody(this->_response_body);
}

bool CgiHandler::splitHeadersAndBody()
{
	//+ Empty CGI output
	if (this->_readBuffer.empty())
		return false;

	//+ Find header/body separator
	size_t sep = this->_readBuffer.find("\r\n\r\n");
	size_t sep_len = 4;

	if (sep == std::string::npos)
	{
		sep = this->_readBuffer.find("\n\n");
		sep_len = 2;
	}

	//+ CGI did not send complete headers
	if (sep == std::string::npos)
		return false;

	//+ Make the split
	this->_response_headers = this->_readBuffer.substr(0, sep);
	this->_response_body = this->_readBuffer.substr(sep + sep_len);

	return true;
}

bool CgiHandler::parseResponseHeaders()
{
	//+ Default CGI response
	int status_code = 200;
	std::string status_message = "OK";

	bool has_content_type = false;

	//+ Parse headers
	std::istringstream header_stream(this->_response_headers);
	std::string line;

	while (std::getline(header_stream, line))
	{
		//+ Remove CR from CRLF lines
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		//+ Ignore empty lines
		if (line.empty())
			continue;

		size_t colon = line.find(':');

		//+ Invalid header
		if (colon == std::string::npos)
			return false;

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		//+ Trim spaces after ':'
		while (!value.empty() && value[0] == ' ')
			value.erase(0, 1);

		//+ Trim trailing spaces
		while (!value.empty() && value[value.size() - 1] == ' ')
			value.erase(value.size() - 1);

		//+ Status header
		if (key == "Status")
		{
			std::istringstream status(value);

			if (!(status >> status_code))
				return false;

			std::getline(status, status_message);

			if (!status_message.empty() && status_message[0] == ' ')
				status_message.erase(0, 1);
		}
		else
		{
			this->_http_response.setHeader(key, value);

			if (key == "Content-Type")
				has_content_type = true;
		}
	}

	//+ Normal CGI responses need Content-Type
	if (!has_content_type && status_code < 300)
		return false;

	//+ Populate HttpResponse
	this->_http_response.setStatusCode(status_code);
	this->_http_response.setStatusMessage(status_message);

	return true;
}

void	CgiHandler::closeParentFdOut() { close(this->_parentFdOut); this->_parentFdOut = -1; }

void	CgiHandler::setWriteBuffer(std::string str) { this->_writeBuffer = str; }

// ---------- GETTERS ----------

pid_t	CgiHandler::getPid() const { return this->_pid; }

int	CgiHandler::getParentFdIn() const { return this->_parentFdIn; }

int	CgiHandler::getParentFdOut() const { return this->_parentFdOut; }

bool	CgiHandler::isFinished() const { return this->_finished; }

bool	CgiHandler::finishedWriting() const { return this->_finishedWritting; }

const std::string&	CgiHandler::getWriteBuffer() const { return this->_writeBuffer; }

size_t	CgiHandler::getWriteOffset() const { return this->_writeOffset; }

const std::string&	CgiHandler::getReadBuffer() const { return this->_readBuffer; }

std::vector<char*>	CgiHandler::getEnv() const { return this->_env; }

time_t	CgiHandler::getStartTime() const { return this->_startTime; }

int	CgiHandler::getExitStatus() const { return this->_exitStatus; }

const std::string&	CgiHandler::getScriptPath() const { return this->_scriptPath; }

Client*	CgiHandler::getClient() const { return this->_client; }

int CgiHandler::getStdinFd() const { return this->_parentFdOut; }

int CgiHandler::getStdoutFd() const { return this->_parentFdIn; }


// ---------- MONITORING ----------


void	CgiHandler::updateCgiTime() { time(&(this->_startTime)); }

bool	CgiHandler::cgiTimeout()
{
	time_t	now;
	time(&now);

	if (difftime(now, this->_startTime) > CGI_TIME_LIMIT)
		return (true);
	return(false);
}
