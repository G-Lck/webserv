#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "WebServ.hpp"

class Client {
	private:
		int			_fd;
		std::string	_read_buffer;
		std::string	_write_buffer;

	public:
		Client();
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();

	const std::string&	getReadBuffer()	const;
	const std::string&	getWriteBuffer()	const;

	void	appendToReadBuffer(const char* data, size_t len);
	void	appendToWriteBuffer(const std::string& data);
	void	eraseWriteBuffer(size_t n);
	void	clearReadBuffer();
	void	clearWriteBuffer();

	};

#endif