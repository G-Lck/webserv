#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "WebServ.hpp"
#include "LocationConfig.hpp"

/**
 * @brief the type of privates attributs are maybe a little bit random
 */
 
class ServerConfig {
	private:
		std::vector< std::pair<std::string, std::string> >	_listen;
		std::vector<std::string>							_server_name;
		std::string											_root;
		std::vector<std::string>							_error_page;
		bool												_autoindex;
		std::vector<LocationConfig>							_locations;
		int													_client_max_body_size;
		std::string											_cgi_handler;

	public:
		ServerConfig();
		ServerConfig(const ServerConfig& other);
		ServerConfig& operator=(const ServerConfig& other);
		~ServerConfig();

		//+ ---- Getters ----
		const std::pair<std::string, std::string>&					getListen(unsigned int i) const;
		const std::vector< std::pair<std::string, std::string> >&	getAllListen( void ) const;
		const std::vector<std::string>&								getServerName( void ) const;
		const std::string&											getRoot( void ) const;
		const std::vector<std::string>&								getErrorPage( void ) const;
		bool														getAutoindex( void ) const;
		const std::vector<LocationConfig>&							getLocations( void ) const;
		int															getClientMaxBodySize( void ) const;
		const std::string&											getCgiHandler( void ) const;

		//+ ---- Setters ----
		void    addListen( const std::string &host, const std::string &port );
		void    addServerName( const std::string &name );
		void    setRoot( const std::string &root );
		void    addErrorPage( const std::string &error_page );
		void    setAutoindex( bool autoindex );
		void    addLocation( const LocationConfig &location );
		void    setClientMaxBodySize( int size );
		void    setCgiHandler( const std::string &cgi_handler );
};

#endif