#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "WebServ.hpp"
#include "HttpRequest.hpp"

class HttpRequest;

class Client
{
	private:
		int						_fd;
		time_t 					_last_activity;
		std::string				_read_buffer;
		std::string				_write_buffer;
		HttpRequest				_request;
		std::deque<std::string> _response_queue;

		Client(const Client& other);
		Client& operator=(const Client& other);

	public:

		Client();
		Client( int fd );
		~Client();

	const std::string&	getReadBuffer()		const;
	const std::string&	getWriteBuffer()	const;
	const HttpRequest&	getRequest()	const;

	void	appendToReadBuffer(const char* data, size_t len);
	void	appendToWriteBuffer(const std::string& data);
	void	eraseWriteBuffer(size_t n);
	void	clearReadBuffer();
	void	eraseFromReadBuffer(size_t n);
	void	clearWriteBuffer();
	bool	parseBufferedRequest();
	void	resetCurrentRequest();
	void	addResponseQueue(const std::string& response);
	bool	isResponseQueueEmpty()	const;
	const std::string&	frontResponse() const;
	void	popFrontResponse();
	void	closeFd(void);
};

#endif