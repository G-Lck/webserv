#ifndef PARSECONFIG_HPP
# define PARSECONFIG_HPP

#include "WebServ.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

enum GlobalToken 
{
	G_ROOT,
	G_INDEX,
	G_ERROR_PAGE,
	G_AUTOINDEX,
	G_MAX_BODY,
	G_SERVER,
	G_UNKNOWN
};
enum ServerToken 
{
	S_LISTEN,
	S_SERVER_NAME,
	S_ROOT,
	S_ERROR_PAGE,
	S_AUTOINDEX,
	S_MAX_BODY,
	S_CGI,
	S_LOCATION,
	S_UNKNOWN
};

enum LocationToken 
{
	L_ROOT,
	L_INDEX,
	L_ERROR_PAGE,
	L_AUTOINDEX,
	L_MAX_BODY,
	L_CGI,
	L_LIMIT_EXCEPT,
	L_RETURN,
	L_UNKNOWN
};

class ParseConfig
{
	private:
		std::vector<std::string>	_tokens;

		ParseConfig(const ParseConfig &other);
		ParseConfig &operator=(const ParseConfig &other);

		//+ ---- Parsing Setup ----
		std::string	getFileText( const char* dir_c );
		void 		tokenizer(std::string fileText);

		//+ ---- Recursive Parsing ----		
		void		parseConfig(std::vector<std::string>::iterator &it, GlobalConfig &config);
		void		parseLocation(std::vector<std::string>::iterator &it, GlobalConfig &config, LocationConfig &locConfig);
		void		parseServer(std::vector<std::string>::iterator &it, GlobalConfig &config, ServerConfig &servConfig);

		//+ ---- Parse Helpers ----
		int		getGlobalType(std::string& token);
		int		getServerType(std::string& token);
		int		getLocationType(std::string &token);
		void	parseGlobalToken(std::vector<std::string>::iterator &it, GlobalConfig &config);
		void	parseServerToken(std::vector<std::string>::iterator &it, ServerConfig &config);
		void	parseLocationToken(std::vector<std::string>::iterator &it, LocationConfig &config);

		//+ ---- Cases ----
		void	rootCase(std::vector<std::string>::iterator &it, GlobalConfig &config);
		void	indexCase(std::vector<std::string>::iterator &it, GlobalConfig &config);
		void	errorPagesCase(std::vector<std::string>::iterator &it, GlobalConfig &config);
		void	autoindexCase(std::vector<std::string>::iterator &it, GlobalConfig &config);
		void	maxBodyCase(std::vector<std::string>::iterator &it, GlobalConfig &config);

		void	servRootCase(std::vector<std::string>::iterator &it, ServerConfig &config);
		void	servErrorPagesCase(std::vector<std::string>::iterator &it, ServerConfig &config);
		void	servAutoindexCase(std::vector<std::string>::iterator &it, ServerConfig &config);
		void	servMaxBodyCase(std::vector<std::string>::iterator &it, ServerConfig &config);
		void	servListenCase(std::vector<std::string>::iterator &it, ServerConfig &config);
		void	servCgiCase(std::vector<std::string>::iterator &it, ServerConfig &config);
		void	servServerNameCase(std::vector<std::string>::iterator &it, ServerConfig &config);

	public:
		~ParseConfig();
		ParseConfig();
		ParseConfig( const char* dir );

		void	parse(GlobalConfig &config);

		class 	ParseErrException : public std::runtime_error 
		{
			public:
				ParseErrException(const char* message);
		};
};

#endif