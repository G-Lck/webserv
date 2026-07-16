#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "WebServ.hpp"
#include "HttpRequest.hpp"

class HttpRequest;

class Client
{
	private:
		int									_fd;
		std::pair<std::string, std::string>	_host_port;
		std::string							_read_buffer;
		std::string							_write_buffer;
		size_t								_write_offset;
		HttpRequest							_request;
		std::deque<std::string>				_response_queue;
		time_t 								_last_activity;
		bool								_keep_alive;

		Client(const Client& other);
		Client& operator=(const Client& other);
	public:

		Client();
		Client(int fd, std::pair<std::string, std::string>	host_port);
		~Client();

	const std::string&					getReadBuffer()		const;
	const char*							getWriteData() 		const;
	size_t								getWriteRemaining() const;
	const HttpRequest&					getRequest()		const;
	time_t								getLastActivity()	const;
	int									getFd()				const;
	std::pair<std::string, std::string>	getHostPort() 		const;

	void				appendToReadBuffer(const char* data, size_t len);
	void				appendToWriteBuffer(const std::string& data);

	void				eraseWriteBuffer(size_t n);
	void				eraseProcessedRequest();
	void				clearReadBuffer();
	void				eraseFromReadBuffer(size_t n);
	void				clearWriteBuffer();
	bool				parseBufferedRequest();

	void				resetCurrentRequest();
	void				addResponseQueue(const std::string& response);
	bool				isResponseQueueEmpty()	const;
	const std::string&	frontResponse() const;
	void				popFrontResponse();
	void				closeFd(void);
	bool				isWriteBufferEmpty() const;
	bool				getKeepAlive() const;
	void				setKeepAlive( bool alive );
	void				wantsKeepAlive();
	void				updateTime();
};

std::ostream &operator<<( std::ostream &out, Client const &c );

#endif