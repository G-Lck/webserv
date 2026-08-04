#ifndef STATICHANDLER_HPP
#define STATICHANDLER_HPP

#include "WebServ.hpp"
#include "Handler.hpp"

class StaticHandler: public Handler
{
	private:
		int			_status_code;
		std::string	_response_body;
		std::string	_content_type;
		StaticHandler(const StaticHandler& other);
	public:
		StaticHandler();
		StaticHandler& operator=(const StaticHandler& other);
		StaticHandler(const Handler& other);
		~StaticHandler();
		
		void	Get();
		void	Post();
		void	Delete();
		
		// Get //
		void	GetDirectory();
		void	GetFile();

		// Post //
		void	parseMultipartData();
		
		// Delete //
		void	CanDelete();

		int				getStatusCode() const;
		const std::string&	getResponseBody() const;
		const std::string&	getContentType() const;
};

#endif