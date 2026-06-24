#ifndef GLOBALCONFIG_HPP
#define GLOBALCONFIG_HPP

#include "ServerConfig.hpp"
#include "WebServ.hpp"

class GlobalConfig
{
    private:
		std::vector<ServerConfig>	_servers;
		std::string					_root;
		std::vector<std::string>	_index;
		std::map<int, std::string>	_error_pages;
		bool						_autoindex;
		int							_client_max_body_size;

	public:
		GlobalConfig();
		GlobalConfig(const GlobalConfig& other);
		GlobalConfig& operator=(const GlobalConfig& other);
		~GlobalConfig();

		//+ --- Vector Management ---
		size_t				serverCount( void ) const;
		const ServerConfig&	getServers( size_t i ) const;

		//+ --- Getters for Defaults ---
		const std::string&					getRoot( void ) const;
		const std::vector<std::string>&		getIndex( void ) const;
		const std::map<int, std::string>&	getErrorPages( void ) const;
		bool								getAutoindex( void ) const;
		int									getClientMaxBodySize( void ) const;
		
		//+ --- Setters ---
		void	addServer( ServerConfig newServ );
		void	setRoot( const std::string &root );
		void	addIndex( const std::string &index );
		void	addErrorPage( int code, const std::string &path );
		void	setAutoindex( bool autoindex );
		void	setClientMaxBodySize( int size );
};

#endif