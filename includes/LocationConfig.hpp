#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include "WebServ.hpp"

/**
 * @brief the type of privates attributs are maybe a little bit random
 */
class LocationConfig {

	private:
		std::string							_root;
		std::string							_path;
		bool								_autoindex;
		int									_client_max_body_size;
		std::vector<std::string>			_index;
		std::vector<std::string>			_allow_methods;
		std::map<int, std::string>			_error_pages;
		std::pair<std::string, std::string>	_cgi_handler;
		std::vector<std::string>			_limit_except;
		std::string							_return_url;
		int									_return_code;

		LocationConfig& operator=(const LocationConfig& other);
	public:
		LocationConfig();
		LocationConfig(const LocationConfig& other);
		~LocationConfig();

		//+ ---- Setters ----
		void    setRoot( const std::string &root );
		void    setPath( const std::string &path );
		void    setAutoindex( bool autoindex );
		void    setClientMaxBodySize( int size );
		void    setIndex( const std::vector<std::string> &index );
		void    setAllowMethods( const std::vector<std::string> &methods );
		void    addErrorPage( int code, const std::string &path );
		void    setCgiHandler( const std::pair<std::string, std::string> &cgi_handler );
		void    setLimitExcept( const std::vector<std::string> &limit_except );
		void    setReturn( int code, const std::string &url );

		//+ ---- Getters ----
		const std::string&							getRoot( void ) const;
		const std::string&							getPath( void ) const;
		bool										getAutoindex( void ) const;
		int											getClientMaxBodySize( void ) const;
		const std::vector<std::string>&				getIndex( void ) const;
		const std::vector<std::string>&				getAllowMethods( void ) const;
		const std::map<int, std::string>&			getErrorPages( void ) const;
		const std::pair<std::string, std::string>&	getCgiHandler( void ) const;
		const std::vector<std::string>&				getLimitExcept( void ) const;
		int											getReturnCode( void ) const;
		const std::string&							getReturnUrl( void ) const;
};

#endif