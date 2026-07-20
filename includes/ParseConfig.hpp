#ifndef PARSECONFIG_HPP
# define PARSECONFIG_HPP

#include "WebServ.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

enum e_token_type 
{
	T_ROOT,
	T_INDEX,
	T_ERROR_PAGE,
	T_AUTOINDEX,
	T_MAX_BODY,
	T_SERVER,
	T_LISTEN,
	T_SERVER_NAME,
	T_CGI,
	T_LOCATION,
	T_LIMIT_EXCEPT,
	T_ALLOW_METHODS,
	T_RETURN,
	T_UPLOAD_PATH,
	T_UNKNOWN
};

class ParseConfig
{
	private:
		std::vector<std::string>    _tokens;

		ParseConfig(const ParseConfig &other);
		ParseConfig &operator=(const ParseConfig &other);

		//+ ---- Parsing Setup ----
		std::string getFileText( const char* dir_c );
		void        tokenizer(std::string fileText);

		//+ ---- Recursive Parsing ----     
		void        parseConfig(std::vector<std::string>::iterator &it, GlobalConfig &config);
		void        parseLocation(std::vector<std::string>::iterator &it, GlobalConfig &config, LocationConfig &locConfig);
		void        parseServer(std::vector<std::string>::iterator &it, GlobalConfig &config, ServerConfig &servConfig);

		//+ ---- Validate dirs ----
		void		validatePaths( GlobalConfig &config ) const;

		//+ ---- Check for default ----
		void		applyDefaults(GlobalConfig &config);

		//+ ---- Parse Helpers ----
		int     getGlobalType(std::string& token);
		int     getServerType(std::string& token);
		int     getLocationType(std::string &token);
		void    parseGlobalToken(std::vector<std::string>::iterator &it, GlobalConfig &config);
		void    parseServerToken(std::vector<std::string>::iterator &it, ServerConfig &config);
		void    parseLocationToken(std::vector<std::string>::iterator &it, LocationConfig &config);

		//+ ---- Shared Cases (Templates) ----
		template <typename T>
		void    parseRoot(std::vector<std::string>::iterator &it, T &config);

		template <typename T>
		void    parseIndex(std::vector<std::string>::iterator &it, T &config);

		template <typename T>
		void    parseErrorPages(std::vector<std::string>::iterator &it, T &config);

		template <typename T>
		void    parseAutoindex(std::vector<std::string>::iterator &it, T &config);

		template <typename T>
		void    parseMaxBody(std::vector<std::string>::iterator &it, T &config);

		template <typename T>
		void    parseCGI(std::vector<std::string>::iterator &it, T &config);

		//+ ---- Server Specific Cases ----
		void    servParseListen(std::vector<std::string>::iterator &it, ServerConfig &config);
		void    servParseServerName(std::vector<std::string>::iterator &it, ServerConfig &config);

		//+ ---- Location Specific Cases ----
		void	locParseAllowedMethods(std::vector<std::string>::iterator &it, LocationConfig &config);
		void	locParseReturnCode(std::vector<std::string>::iterator &it, LocationConfig &config);
		void	locParseUploadPath(std::vector<std::string>::iterator &it, LocationConfig &config);

	public:
		~ParseConfig();
		ParseConfig();
		ParseConfig( const char* dir );

		void    parse(GlobalConfig &config);

		class   ParseErrException : public std::runtime_error 
		{
			public:
				ParseErrException(const char* message);
		};
};

bool		isValidFile(const std::string& path, int flag);

#endif