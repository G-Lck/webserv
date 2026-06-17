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

void	Client::clearWriteBuffer()
{
	this->_write_buffer.clear();
}