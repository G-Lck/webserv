#ifndef STATICHANDLER_HPP
#define STATICHANDLER_HPP

#include "WebServ.hpp"
#include "Handler.hpp"

class StaticHandler: public Handler
{
	public:
		StaticHandler();
		StaticHandler(const StaticHandler& other);
		StaticHandler& operator=(const StaticHandler& other);
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
};

#endif