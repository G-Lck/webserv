#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include "WebServ.hpp"

/**
 * @brief the type of privates attributs are maybe a little bit random
 */
class LocationConfig {

	private:
		std::string					_root;
		std::string 				_path;
		bool						_autoindex;
		int							_client_max_body_size;
		std::vector<std::string>	_index;
		std::vector<std::string>	_error_pages;
		std::string					_cgi_handler;
		std::string					_limit_except;
		std::string					_return;

	public:
		LocationConfig();
		LocationConfig(const LocationConfig& other);
		LocationConfig& operator=(const LocationConfig& other);
		~LocationConfig();

		//+ ---- Setters ----
		void	setRoot( const std::string &root );
		void	setPath( const std::string &path );
		void	setAutoindex( bool autoindex );
		void	setClientMaxBodySize( int size );
		void	setIndex( const std::vector<std::string> &index );
		void	setErrorPages( const std::vector<std::string> &error_pages );
		void	setCgiHandler( const std::string &cgi_handler );
		void	setLimitExcept( const std::string &limit_except );
		void	setReturn( const std::string &ret );

		//+ ---- Getters ----
		const std::string&				getRoot( void ) const;
		const std::string&				getPath( void ) const;
		bool							getAutoindex( void ) const;
		int								getClientMaxBodySize( void ) const;
		const std::vector<std::string>&	getIndex( void ) const;
		const std::vector<std::string>&	getErrorPages( void ) const;
		const std::string&				getCgiHandler( void ) const;
		const std::string&				getLimitExcept( void ) const;
		const std::string&				getReturn( void ) const;
};

#endif