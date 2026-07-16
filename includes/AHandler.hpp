#ifndef AHANDLER_HPP
#define AHANDLER_HPP

class AHandler
{
	private:
		Client			_client;
		HttpRequest		_http_request;
		HttpResponse*	_http_response;
		GlobalConfig	_global_config;
		LocationConfig	_location;
		std::string		_method;
		std::string		_path;

		void	findLocation();
		void	checkRedirection();
		void	checkMethod();
		void	checkValidPath();
		void	constructPath();

	public:
		AHandler();
		AHandler(const AHandler& other);
		AHandler& operator=(const AHandler& other);
		~AHandler();
		void	run(); // should be virtual
};

#endif