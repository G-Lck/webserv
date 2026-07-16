#include "../../includes/Handler.hpp"

Handler::Handler() {}

Handler::Handler(Client client, const std::vector<ServerConfig> &virtual_servers): _client(client), _virtual_servers(virtual_servers) {}

Handler::Handler(const Handler& other) { *this = other; }

Handler& Handler::operator=(const Handler& other) {
	if (this != &other)
	{
		this->_client = other._client;
		this->_http_request = other._http_request;
		this->_http_response = other._http_response;
		this->_virtual_servers = other._virtual_servers;
		this->_my_virtual_server = other._my_virtual_server;
		this->_location = other._location;
		this->_method = other._method;
		this->_path = other._path;
	}
	return *this;
}

Handler::~Handler() {}

/// @brief findServer will try to match the server name, if given, by the first virtual server with
/// this name. If no name or not found we fall back on the first server with default_server as name.
/// If no default_server is found, we take the first server in the list. If no server is configured
/// we throw a 500 error (shouldn't happen if the config is valid)
void	Handler::findServer()
{
	std::string name = "default_server";
	std::map<std::string, std::string> headers = this->_http_request.getHeaders();
	std::map<std::string, std::string>::const_iterator host_it = headers.find("Host");

	if (host_it != headers.end())
	{
		name = host_it->second;
		size_t colon_pos = name.find(':');
		if (colon_pos != std::string::npos)
			name = name.substr(0, colon_pos);
	}

	for (size_t i = 0; i < this->_virtual_servers.size(); i++)
	{
		const std::vector<std::string> &server_names = this->_virtual_servers[i].getServerName();
		for (size_t j = 0; j < server_names.size(); j++)
		{
			if (server_names[j] == name)
			{
				this->_my_virtual_server = this->_virtual_servers[i];
				return ;
			}
		}
	}

	for (size_t i = 0; i < this->_virtual_servers.size(); i++)
	{
		const std::vector<std::string> &server_names = this->_virtual_servers[i].getServerName();
		for (size_t j = 0; j < server_names.size(); j++)
		{
			if (server_names[j] == "default_server")
			{
				this->_my_virtual_server = this->_virtual_servers[i];
				return ;
			}
		}
	}

	if (!this->_virtual_servers.empty())
		this->_my_virtual_server = this->_virtual_servers[0];
	else
		throw HttpException(500, "No virtual servers configured");
}

/// @brief find the location with the longest path inside all location of the virtual server.
/// if not found, throw a 404 error.
void	Handler::findLocation()
{
	if (this->_my_virtual_server.getLocations().empty())
		throw HttpException(500, "No locations configured for this virtual server");

	const std::vector<LocationConfig> &locations = this->_my_virtual_server.getLocations();
	size_t longest_match_length = 0;
	for (size_t i = 0; i < locations.size(); i++)
	{
		const std::string &location_path = locations[i].getPath();
		if (this->_path.compare(0, location_path.length(), location_path) == 0)
		{
			if (location_path.length() > longest_match_length)
			{
				longest_match_length = location_path.length();
				this->_location = locations[i];
			}
		}
	}
	if (longest_match_length == 0)
		throw HttpException(404, "Not Found");
}

/// @brief check if this location has a redirection and throw it.
void	Handler::checkRedirection()
{
	if (this->_location.getReturnCode() != 0)
		throw HttpException(this->_location.getReturnCode(), this->_location.getReturnUrl());
}

/// @brief check if the method of the HttpRequest is allowed in this location.
void	Handler::checkMethod()
{
	const std::vector<std::string> &allowed_methods = this->_location.getAllowMethods();
	if (std::find(allowed_methods.begin(), allowed_methods.end(), this->_method) == allowed_methods.end())
		throw HttpException(405, "Method Not Allowed");
}

/// @brief ../ is forbidden in order to avoid back attack. And because we don't need to do
/// the regex for the path I think this is enough.
void	Handler::checkDotsPath()
{
	if (this->_http_request.getPath().find("..") != std::string::npos)
		throw HttpException(418, "I'm a teapot");
}

void	Handler::constructPath()
{
	this->_path = "." + this->_location.getRoot() + this->_http_request.getPath();
}
///
void	Handler::run()
{
	checkDotsPath();
	findServer();
	findLocation();
	checkRedirection();
	checkMethod();
	constructPath();
}
