#include "../../includes/HandlerCGI.hpp"

// ---------- ORTHODOX ----------

CgiHandler::CgiHandler() : _pid(-1), _stdinFd(-1), _stdoutFd(-1), _finished(false), _startTime(time(NULL)) { }

CgiHandler::~CgiHandler()
{
	// kill pid???
	if (this->_stdinFd != -1)
		close(this->_stdinFd);
	if (this->_stdoutFd != -1)
		close(this->_stdoutFd);

	if (this->_env != NULL)
	{
		for (size_t i = 0; this->_env[i] != NULL; i++)
			delete[] this->_env[i];
		delete[] this->_env;
	}
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
	this->_env = NULL;
	this->_writeOffset = 0;
}

// ---------- VALIDATE REQUEST ----------

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

/// @brief	Function to validate the content length header
///			It will check its there for a second time
///			also checks and most important, if is not bigger than max size
///			And also compares with the size of the body to actually match
bool	CgiHandler::validContentLength()
{
	const std::map<std::string, std::string> headers = this->getHttpRequest().getHeaders();
	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");

	if (it == headers.end()) //+ Extra check if not found (we check that in the parser anyways)
		throw HttpException(411, "The request did not specify the length of its content");

	size_t content_length = std::atoi(it->second.c_str()); //+ Get the length set in the header

	if (content_length != this->getHttpRequest().getBody().size()
		|| content_length > (size_t)this->getLocation().getClientMaxBodySize())
		return false; //+ Check if size is not bigger than location or matches with the actual body
	return (true);
}

/// @brief	Checks comnparing the method passed in the request with the allowed methos for that location
bool	CgiHandler::validMethod()
{
	const std::vector<std::string> allowed_methods = this->_location.getAllowMethods();
	std::vector<std::string>::const_iterator it = allowed_methods.begin();
	
	while (it != allowed_methods.end())
	{
		if (*it == this->_client->getRequest().getMethod())
			return true;
		it++;
	}
	return (false);
}

// ---------- SETUP ----------

void	CgiHandler::setEnvVars()
{

}

void	CgiHandler::openPipe()
{

}

// ---------- GETTERS ----------

pid_t	CgiHandler::getPid() const { return this->_pid; }

int	CgiHandler::getStdinFd() const { return this->_stdinFd; }

int	CgiHandler::getStdoutFd() const { return this->_stdoutFd; }

bool	CgiHandler::isFinished() const { return this->_finished; }

const std::string&	CgiHandler::getWriteBuffer() const { return this->_writeBuffer; }

size_t	CgiHandler::getWriteOffset() const { return this->_writeOffset; }

const std::string&	CgiHandler::getReadBuffer() const { return this->_readBuffer; }

char**	CgiHandler::getEnv() const { return this->_env; }

time_t	CgiHandler::getStartTime() const { return this->_startTime; }

int	CgiHandler::getExitStatus() const { return this->_exitStatus; }

const std::string&	CgiHandler::getScriptPath() const { return this->_scriptPath; }

Client*	CgiHandler::getClient() const { return this->_client; }
