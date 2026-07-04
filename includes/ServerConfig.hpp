#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "WebServ.hpp"
#include "LocationConfig.hpp"
#include "Types.hpp"

/**
 * @brief the type of privates attributs are maybe a little bit random
 */
 
class ServerConfig {
	private:
		t_port_host							_listen;
		std::vector<std::string>			_server_name;
		std::vector<std::string>			_index;
		std::string							_root;
		std::map<int, std::string>			_error_pages;
		bool								_autoindex;
		std::vector<LocationConfig>			_locations;
		int									_client_max_body_size;
		std::pair<std::string, std::string>	_cgi_handler;

		ServerConfig& operator=(const ServerConfig& other);
	public:
		ServerConfig();
		ServerConfig(const ServerConfig& other);
		~ServerConfig();

		//+ ---- Getters ----
		const std::pair<std::string, std::string>&  getListen( int pos ) const;
		const t_port_host&                          getAllListen( void ) const;
		const std::vector<std::string>&             getServerName( void ) const;
		const std::vector<std::string>&             getIndex( void ) const;
		const std::string&                          getRoot( void ) const;
		const std::map<int, std::string>&           getErrorPage( void ) const; 
		bool                                        getAutoindex( void ) const;
		const std::vector<LocationConfig>&          getLocations( void ) const;
		int                                         getClientMaxBodySize( void ) const;
		const std::pair<std::string, std::string>&  getCgiHandler( void ) const;

		//+ ---- Setters ----
		void    addListen( const std::string &host, const std::string &port );
		void    addServerName( const std::string &name );
		void    addIndex( const std::string &index );
		void    setRoot( const std::string &root );
		void    addErrorPage( int code, const std::string &path );
		void    setAutoindex( bool autoindex );
		void    addLocation( const LocationConfig &location );
		void    setClientMaxBodySize( int size );
		void    setCgiHandler( const std::pair<std::string, std::string> &cgi_handler );
};

#endif