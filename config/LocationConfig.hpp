#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

/**
 * @brief the type of privates attributs are maybe a little bit random
 */
class LocationConfig {

	private:
		std::string			_root;
		bool				_autoindex;
		int					_client_max_body_size;
		std::vector<int>	index;
		std::vector<std::string>	_error_pages;
		std::string			_cgi_handler;
		std::string			_limit_except;
		std::string			_return;

	public:
		LocationConfig();
		LocationConfig(const LocationConfig& other);
		LocationConfig& operator=(const LocationConfig& other);
		~LocationConfig();
};

#endif