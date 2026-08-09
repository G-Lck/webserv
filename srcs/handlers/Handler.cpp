#include "../../includes/Handler.hpp"

// ---------- ORTHODOX ----------

Handler::Handler() {}

Handler::Handler(Client* client): _client(client)
{
	HttpRequest req = client->getRequest(); 
	this->_http_request = req;
}

Handler::Handler(const Handler& other) { *this = other; }

Handler& Handler::operator=(const Handler& other) {
	if (this != &other)
	{
		this->_client = other._client;
		this->_http_request = other._http_request;
		this->_my_virtual_server = other._my_virtual_server;
		this->_location = other._location;
		this->_method = other._method;
		this->_path = other._path;
	}
	return *this;
}

Handler::~Handler() {}


// ---------- DEFINE TYPE ----------


/// @brief	Function that compares the _path set on the Handler during setup and the 
///			cgi_path set in the LocationConfig for the running virtual sever.
/// @return If the location is not empty, and it matches with the one in location config, it will
///			return true, otherwise returns false.
bool	Handler::isCGI() const
{
	const std::string &cgi_path = this->_location.getCgiHandler().first;
	if (cgi_path.empty())
		return false;
	if (this->_path.length() < cgi_path.length())
		return false;
	if (this->_path.compare(this->_path.length() - cgi_path.length(), cgi_path.length(), cgi_path) == 0)
		return true;
	return (false);
}  //~ CHECK THIS

// ---------- SETUP/PARSE FUNCTIONS ----------


/// @brief findServer will try to match the server name, if given, by the first virtual server with
/// this name. If no name or not found we fall back on the first server with default_server as name.
/// If no default_server is found, we take the first server in the list. If no server is configured
/// we throw a 500 error (shouldn't happen if the config is valid)
void	Handler::findServer(const std::vector<ServerConfig> &virtual_servers)
{
	std::string name = "default_server";
	std::map<std::string, std::string> headers = this->_http_request.getHeaders();
	std::map<std::string, std::string>::const_iterator host_it = headers.find("host");

	if (host_it != headers.end())
	{
		name = host_it->second;
		size_t colon_pos = name.find(':');
		if (colon_pos != std::string::npos)
			name = name.substr(0, colon_pos);
	}

	//+ Find matching server by name, set and return
	for (size_t i = 0; i < virtual_servers.size(); i++)
	{
		const std::vector<std::string> &server_names = virtual_servers[i].getServerName();
		for (size_t j = 0; j < server_names.size(); j++)
		{
			if (server_names[j] == name)
			{
				this->_my_virtual_server = virtual_servers[i];
				this->_server_name = name;
				return ;
			}
		}
	}

	//+ Find matching server by default_server if no match with server name, set and return
	for (size_t i = 0; i < virtual_servers.size(); i++)
	{
		const std::vector<std::string> &server_names = virtual_servers[i].getServerName();
		for (size_t j = 0; j < server_names.size(); j++)
		{
			if (server_names[j] == "default_server")
			{
				this->_my_virtual_server = virtual_servers[i];
				this->_server_name = name;
				return ;
			}
		}
	}

	//+ If no match was found set default or thrown if there are no servers
	if (!virtual_servers.empty())
	{
		this->_my_virtual_server = virtual_servers[0];
		this->_server_name = name;
	}
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
		if (this->_http_request.getPath().compare(0, location_path.length(), location_path) == 0)
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
	{
		this->setResponseHeader("Location", this->_location.getReturnUrl());
		throw HttpException(this->_location.getReturnCode(), "Redirect");
	}
}

/// @brief check if the method of the HttpRequest is allowed in this location.
void	Handler::checkMethod()
{
	this->_method = this->_http_request.getMethod();
	const std::vector<std::string> &allowed_methods = this->_location.getAllowMethods();

	if (std::find(allowed_methods.begin(), allowed_methods.end(), this->_method) != allowed_methods.end())
		return;

	std::string allow_value;
	for (size_t i = 0; i < allowed_methods.size(); ++i)
	{
		if (i > 0)
			allow_value += ", ";
		allow_value += allowed_methods[i];
	}

	this->setResponseHeader("Allow", allow_value);
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
void	Handler::initAndParseHandler(const std::vector<ServerConfig> &virtual_servers)
{
	checkDotsPath();
	findServer(virtual_servers);
	findLocation();
	validateBodySize();
	checkRedirection();
	checkMethod();
	constructPath();
}


// ---------- VALIDATE REQUEST ----------


/// @brief	Fucntion to validate the request values before starting with the cgi
///			checks: isValidFile(), validMethod(), validContentLength()
void	Handler::validateStatic()
{
	if (!validMethod())	//+ Check if allowed methods in location matches with request method
		throw HttpException(405, "Method Not Allowed");
	if (!validContentLength())
		throw HttpException(400, "Bad Request (Content-Length)");
}

/// @brief	Function to validate the content length header
///			It will check its there for a second time
///			also checks and most important, if is not bigger than max size
///			And also compares with the size of the body to actually match
bool	Handler::validContentLength()
{
	const std::map<std::string, std::string> headers = this->getRequestHandler().getHeaders();
	std::map<std::string, std::string>::const_iterator it = headers.find("content-length");

	if (it == headers.end()) //+ Extra check if not found (we check that in the parser anyways)
		throw HttpException(411, "The request did not specify the length of its content");

	size_t content_length = std::atoi(it->second.c_str()); //+ Get the length set in the header

	if (content_length != this->getRequestHandler().getBody().size()
		|| content_length > (size_t)this->getLocation().getClientMaxBodySize())
		return false; //+ Check if size is not bigger than location or matches with the actual body
	return (true);
}

/// @brief	Checks comnparing the method passed in the request with the allowed methos for that location
bool	Handler::validMethod()
{
	const std::vector<std::string> allowed_methods = this->_location.getAllowMethods();
	std::vector<std::string>::const_iterator it = allowed_methods.begin();
	
	while (it != allowed_methods.end())
	{
		if (*it == this->getRequestHandler().getMethod())
			return true;
		it++;
	}
	return (false);
}


// ---------- GETTERS ----------


Client* Handler::getClient() const { return this->_client; }

const HttpRequest& Handler::getRequestHandler() const { return this->_http_request; }

const HttpResponse& Handler::getResponseHandler() const { return this->_http_response; }

const ServerConfig& Handler::getVirtualServer() const { return this->_my_virtual_server; }

const LocationConfig& Handler::getLocation() const { return this->_location; }

const std::string& Handler::getMethod() const { return this->_method; }

const std::string& Handler::getPath() const { return this->_path; }

const std::string&	Handler::getServerName() const { return (this->_server_name); }

const std::map<std::string, std::string>& Handler::getResponseHeaders() const
{
	return this->_response_headers;
}

void	Handler::validateBodySize() const
{
	if (this->_http_request.getBody().size() > static_cast<size_t>(this->_location.getClientMaxBodySize()))
		throw HttpException(413, "Payload Too Large");
}

// ---------- SETTERS ------------

void Handler::setResponseHeader(const std::string &key, const std::string &value)
{
	this->_response_headers[key] = value;
}

// ---------- ERROR PAGES ----------

bool Handler::hasErrorPage(int code) const { return this->_location.getErrorPages().find(code) != this->_location.getErrorPages().end(); }

std::string	Handler::CreateErrorPageContent(int code) const
{
	if (this->hasErrorPage(code))
	{
		std::map<int, std::string>::const_iterator it = this->_location.getErrorPages().find(code);
		std::string error_page_path = "." + this->_location.getRoot() + it->second;
		std::ifstream file(error_page_path.c_str());
		if (file.is_open() && file.good())
		{
			std::ostringstream content;
			content << file.rdbuf();
			return content.str();
		}
	}
	return ""; // should return the default one
}