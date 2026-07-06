#include "../../includes/Client.hpp"

// ------- Orthodox --------

Client::Client() : _fd(-1) { std::cout << "Client default constructor called" << std::endl; }

Client::Client( int fd ) : _fd(fd) { };

Client::~Client() { }

// ------- Getters --------

const std::string&	Client::getReadBuffer()	const
{
	return (this->_read_buffer);
}

const std::string&	Client::getWriteBuffer() const
{
	return (this->_write_buffer);
}

const HttpRequest&	Client::getRequest()	const
{
	return (this->_request);
}

// ------- Member variable manipulation --------

void	Client::appendToReadBuffer(const char* data, size_t len)
{
	this->_read_buffer.append(data, len);
}

void	Client::appendToWriteBuffer(const std::string& data)
{
	this->_write_buffer.append(data);
}

void	Client::eraseWriteBuffer(size_t n)
{
	this->_write_buffer.erase(0, n);
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

void	Client::clearWriteBuffer()
{
	this->_write_buffer.clear();
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
