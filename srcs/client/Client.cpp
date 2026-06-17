#include "../../includes/Client.hpp"

Client::Client()
{
	std::cout << "Client default constructor called" << std::endl;
}

Client::Client(const Client& other)
{
	std::cout << "Client copy constructor called" << std::endl;
	*this = other;
}

Client& Client::operator=(const Client& other)
{
	std::cout << "Client operator = called" << std::endl;
	if (this != &other) {
		this->_fd = other._fd;
		this->_read_buffer = other._read_buffer;
		this->_write_buffer = other._write_buffer;
	}
	return *this;
}

Client::~Client() {}

const std::string&	Client::getReadBuffer()	const
{
	return (this->_read_buffer);
}

const std::string&	Client::getWriteBuffer()	const
{
	return (this->_write_buffer);
}

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