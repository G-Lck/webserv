#ifndef PARSECONFIG_HPP
# define PARSECONFIG_HPP

#include "WebServ.hpp"
#include "GlobalConfig.hpp"

enum GlobalDirective
{	G_ROOT,
	G_INDEX,
	G_ERR_PAGE,
	G_AUTOINDEX,
	G_MAX_BODY,
	G_SERVER
};

enum ServerDirective
{
	S_LISTEN,
	S_NAME,
	S_ROOT,
	S_ERR_PAGE,
	S_AUTOINDEX,
	S_MAX_BODY,
	S_CGI,
	S_LOCATION
};

enum LocationDirective
{
	L_ROOT,
	L_INDEX,
	L_ERR_PAGE,
	L_AUTOINDEX,
	L_MAX_BODY,
	L_CGI,
	L_LIMIT,
	L_RETURN
};

class ParseConfig
{
	private:


		ParseConfig(const ParseConfig &other);
		ParseConfig &operator=(const ParseConfig &other);

		std::string	getFileText(const std::string &dir);
		void 		tokenizer(std::string fileText, GlobalConfig &config);		
	public:
		~ParseConfig();
		ParseConfig();
		void	ParseConfig::parse(const std::string &dir, GlobalConfig &config);
		class ParseErrException : public std::runtime_error 
		{
			public:
				ParseErrException(const char* message);
		};
};

#endif