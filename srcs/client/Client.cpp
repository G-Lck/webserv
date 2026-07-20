#include "../../includes/Client.hpp"

// ------- Orthodox --------

Client::Client() : _fd(-1), _write_offset(0), _last_activity(0) { std::cout << "Client default constructor called" << std::endl; }

Client::Client( int fd, std::pair<std::string, std::string>	host_port) : _fd(fd), _host_port(host_port), _write_offset(0), _last_activity(0){ };

Client::Client(const Client& other){ *this = other; }

Client& Client::operator=(const Client& other)
{
	if (this != &other)
	{
		this->_fd = other._fd;
		this->_host_port = other._host_port;
		this->_read_buffer = other._read_buffer;
		this->_write_buffer = other._write_buffer;
		this->_write_offset = other._write_offset;
		this->_request = other._request;
		this->_response_queue = other._response_queue;
		this->_last_activity = other._last_activity;
		this->_keep_alive = other._keep_alive;
	}
	return *this;
}

Client::~Client() { }

// ------- Getters --------

const std::string&	Client::getReadBuffer()	const
{
	return (this->_read_buffer);
}


size_t Client::getWriteRemaining() const
{
	return _write_buffer.size() - _write_offset;
}

const HttpRequest&	Client::getRequest()	const
{
	return (this->_request);
}

const std::deque<std::string>&	Client::getResponseQueue() const
{
	return (this->_response_queue);
}

// ------- Member variable manipulation --------

void	Client::appendToReadBuffer(const char* data, size_t len)
{
	this->_read_buffer.append(data, len);
}

void Client::appendToWriteBuffer(const std::string& data)
{
    this->_write_buffer.append(data);
}

void Client::eraseProcessedRequest()
{
    size_t consumed = this->_request.getConsumedBytes();
    this->eraseFromReadBuffer(consumed);
}

void Client::eraseWriteBuffer(size_t n)
{
    _write_offset += n;
    if (_write_offset >= _write_buffer.size())
    {
        _write_buffer.clear();
        _write_offset = 0;
    }
}

bool Client::isWriteBufferEmpty() const
{
    return getWriteRemaining() == 0;
}

const char* Client::getWriteData() const
{
    return _write_buffer.data() + _write_offset;
}

void	Client::clearReadBuffer()
{
	this->_read_buffer.clear();
}

void	Client::eraseFromReadBuffer(size_t n)
{
	if (n >= this->_read_buffer.size())
		this->_read_buffer.clear();
	else
		this->_read_buffer.erase(0, n);
}

void Client::clearWriteBuffer()
{
    _write_buffer.clear();
    _write_offset = 0;
}

bool Client::parseBufferedRequest()
{
	return this->_request.parse(this->_read_buffer);
}

void Client::resetCurrentRequest()
{
	this->_request = HttpRequest();
}

void	Client::addResponseQueue(const std::string& response)
{
	this->_response_queue.push_back(response);
}

bool	Client::isResponseQueueEmpty()	const
{
	return(this->_response_queue.empty());
}

const std::string&	Client::frontResponse() const
{
	return(this->_response_queue.front());
}

void	Client::popFrontResponse()
{
	this->_response_queue.pop_front();
}

// --- Keep alive logic ---

bool Client::getKeepAlive() const
{
	return this->_keep_alive;
}

void Client::setKeepAlive( bool alive )
{
	this->_keep_alive = alive;
}

void Client::wantsKeepAlive()
{
	std::map<std::string, std::string>					headers = this->_request.getHeaders();
	std::map<std::string, std::string>::const_iterator	it = headers.find("Connection");
	
	if (it != headers.end())
	{
		if (it->second == "close")
			this->_keep_alive = false;
		else if (it->second == "keep-alive")
			this->_keep_alive = true;
	}
	else
		this->_keep_alive = true;
}

void Client::updateTime()
{
	time(&(this->_last_activity));
}

time_t Client::getLastActivity() const
{
	return (this->_last_activity);
}

/// @brief	Close the file descriptor attached to this Socket.
void	Client::closeFd(void) { if (this->_fd != -1) { close(this->_fd); this->_fd = -1; } }

int	Client::getFd()	const { return this->_fd; }

std::pair<std::string, std::string>	Client::getHostPort() const { return this->_host_port; }

std::ostream &operator<<( std::ostream &out, Client const &c )
{
	time_t last_act = c.getLastActivity();
	char *t = ctime(&last_act);
	t[strlen(t) - 1] = '\0'; //+ remove the new line

	out << "Client [" << c.getFd() << "] " << c.getHostPort().first << ":" << c.getHostPort().second << " Last activity: " << t;
	return out;
}