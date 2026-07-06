#include "../../includes/ParseConfig.hpp"

// ------ Orthodox ------

ParseConfig::ParseConfig() {}
ParseConfig::ParseConfig(const ParseConfig &other) { (void)other; }
ParseConfig &ParseConfig::operator=(const ParseConfig &other) { (void)other; return *this; }
ParseConfig::~ParseConfig() {}
ParseConfig::ParseConfig( const char* dir ) { tokenizer(getFileText(dir)); }

// ------ Exceptions ------

ParseConfig::ParseErrException::ParseErrException(const char* message) : std::runtime_error(message) {}

// ------ PARSE SETUP ------

/// @brief Read the directory and store the content in a string
/// @param dir The directory to read
/// @return The string containing the file text
/// @exception Throws ParseErrExeption in case the file is not found, empty, or has unclosed brackets
std::string	ParseConfig::getFileText( const char* dir_c )
{
	std::ifstream	givenFile(dir_c);
	if (!givenFile.is_open())
		throw ParseErrException("Error\nWrong File");

	std::string	text;
	std::string	buff;

	while (getline (givenFile, buff))
	{
		if (!buff.empty())
		{
			size_t	pos = buff.find_first_not_of(' ');
			if (pos != std::string::npos)
			{
				if (buff[pos] != '#')
					text.append(buff + "\n");
			}
		}
	}
	if (text.empty())
		throw ParseErrException("Error\nEmpty File");
//	if (!validBrackets(text))
//		throw ParseErrException("Error\nUnclosed Brackets {}");
	return (text);
}

/// @brief Tokenizes the text storing it in the vector of string tokens
/// @param fileText The text to tokenize
///	@exception	Throws an exeption for unclosed brackets
void ParseConfig::tokenizer(std::string fileText)
{
	size_t	i = 0;

	while (i < fileText.size())
	{
		std::string token;
		size_t start = fileText.find_first_not_of(" \n", i);
		if (start == std::string::npos)
			break;
		size_t end = fileText.find_first_of(" \n", start);
		if (end == std::string::npos)
			end = fileText.size();
		token = fileText.substr(start, end - start);
		if (token[token.size() - 1] == ';' && token.size() > 1)
		{
			this->_tokens.push_back(token.substr(0, token.size() - 1));
			this->_tokens.push_back(";");
		}
		else if (token[token.size() - 1] == '}' && token.size() > 1)
		{
			this->_tokens.push_back(token.substr(0, token.size() - 1));
			this->_tokens.push_back("}");
		}
		else if (token[0] == '{' && token.size() > 1)
		{
			this->_tokens.push_back("{");
			this->_tokens.push_back(token.substr(1, token.size()));
		}
		else
			this->_tokens.push_back(token);
		i = end;
	}
	// for (std::vector<std::string>::iterator it = _tokens.begin(); it != _tokens.end(); it++)
	// 	std::cout << "[" << *it << "] ";
	std::cout << "\n";
}

// --------- MAIN LOGIC ---------

void ParseConfig::parse(GlobalConfig &config)
{
	std::vector<std::string>::iterator 	it;

	it = this->_tokens.begin();
	parseConfig(it, config);
}

void ParseConfig::parseConfig(std::vector<std::string>::iterator &it, GlobalConfig &config)
{
    if (it == this->_tokens.end())
        return ;

    if (*it == "server")
    {
        it++; // Skip "server"
        if (it == this->_tokens.end() || *it != "{")
            throw ParseErrException("Error: Expected '{' after server");
        it++; // Skip "{"
        
        ServerConfig newServConf;
        parseServer(it, config, newServConf);
        config.addServer(newServConf);
    }
    else
    {
        parseGlobalToken(it, config); 
    }
    parseConfig(it, config);
}

void ParseConfig::parseServer(std::vector<std::string>::iterator &it, GlobalConfig &config, ServerConfig &servConfig)
{
	if (it == this->_tokens.end())
		throw ParseErrException("Error\nUnexpected end of file, missing '}'");
    if (*it == "}")
    {
		it++;
		return ;
    }
	if (*it == "location")
	{
		it++; // Skip "location"
		if ((*it)[0] != '/')
			throw ParseErrException("Error\nWrong path format at location");
		std::string path = *it;
		it++; // Skip path
		if (it == this->_tokens.end() || *it != "{")
			throw ParseErrException("Error\nExpected '{' after location path");
		it++; // Skip "{"
		
		LocationConfig newLocConf;
		newLocConf.setPath(path);
		parseLocation(it, config, newLocConf);
		servConfig.addLocation(newLocConf);
	}
	else
	{
		parseServerToken(it, servConfig);
	}

	parseServer(it, config, servConfig);
}

void ParseConfig::parseLocation(std::vector<std::string>::iterator &it, GlobalConfig &config, LocationConfig &locConfig)
{
	if (it == this->_tokens.end())
		throw ParseErrException("Error: Unexpected end of file, missing '}'");
	if (*it == "}")
	{
		it++;
		return ;
	}
	parseLocationToken(it, locConfig);
	parseLocation(it, config, locConfig);
}

// --------- GET TYPES ---------

int ParseConfig::getGlobalType(std::string& token)
{
	if (token == "root") return T_ROOT;
	if (token == "index") return T_INDEX;
	if (token == "error_page") return T_ERROR_PAGE;
	if (token == "autoindex") return T_AUTOINDEX;
	if (token == "client_max_body_size") return T_MAX_BODY;
	if (token == "server") return T_SERVER;
	return T_UNKNOWN;
}

int ParseConfig::getServerType(std::string& token)
{
	if (token == "listen") return T_LISTEN;
	if (token == "server_name") return T_SERVER_NAME;
	if (token == "index") return T_INDEX;
	if (token == "root") return T_ROOT;
	if (token == "error_page") return T_ERROR_PAGE;
	if (token == "autoindex") return T_AUTOINDEX;
	if (token == "client_max_body_size") return T_MAX_BODY;
	if (token == "cgi_handler") return T_CGI;
	if (token == "location") return T_LOCATION;
	return T_UNKNOWN;
}

int ParseConfig::getLocationType(std::string& token)
{
	if (token == "root") return T_ROOT;
	if (token == "index") return T_INDEX;
	if (token == "error_page") return T_ERROR_PAGE;
	if (token == "autoindex") return T_AUTOINDEX;
	if (token == "client_max_body_size") return T_MAX_BODY;
	if (token == "cgi_handler") return T_CGI;
	if (token == "limit_except") return T_LIMIT_EXCEPT;
	if (token == "allow_methods") return T_ALLOW_METHODS;
	if (token == "return") return T_RETURN;
	if (token == "upload_path") return T_UPLOAD_PATH;
	return T_UNKNOWN;
}

// --------- SWITCHES ---------

void ParseConfig::parseGlobalToken(std::vector<std::string>::iterator &it, GlobalConfig &config)
{
	int type = getGlobalType(*it);

	switch (type)
	{
		case T_ROOT:
			parseRoot(it, config);
			break;
		case T_INDEX:
			parseIndex(it, config);
			break;
		case T_ERROR_PAGE:
			parseErrorPages(it, config);
			break;
		case T_AUTOINDEX:
			parseAutoindex(it, config);
			break;
		case T_MAX_BODY:
			parseMaxBody(it, config);
			break;
		default:
			std::string errMsg = "Error\nInvalid token in global context: " + *it;
			throw ParseErrException(errMsg.c_str());
			break;
	}
}

void ParseConfig::parseServerToken(std::vector<std::string>::iterator &it, ServerConfig &config)
{
	int type = getServerType(*it);

	switch (type)
	{
		case T_LISTEN:
			servParseListen(it, config);
			break;
		case T_SERVER_NAME:
			servParseServerName(it, config);
			break;
		case T_INDEX:
			parseIndex(it, config); 
			break;
		case T_ROOT:
			parseRoot(it, config);
			break;
		case T_ERROR_PAGE:
			parseErrorPages(it, config);
			break;
		case T_AUTOINDEX:
			parseAutoindex(it, config);
			break;
		case T_MAX_BODY:
			parseMaxBody(it, config);
			break;
		case T_CGI:
			parseCGI(it, config);
			break;
		default:
			std::string errMsg = "Error\nInvalid token in server context: " + *it;
			throw ParseErrException(errMsg.c_str());
			break;
	}
}

void ParseConfig::parseLocationToken(std::vector<std::string>::iterator &it, LocationConfig &config)
{
	int type = getLocationType(*it);
	
	switch (type)
	{
		case T_ROOT:
			parseRoot(it, config);
			break;
		case T_INDEX:
			parseIndex(it, config); 
			break;
		case T_ERROR_PAGE:
			parseErrorPages(it, config);
			break;
		case T_AUTOINDEX:
			parseAutoindex(it, config);
			break;
		case T_MAX_BODY:
			parseMaxBody(it, config);
			break;
		case T_CGI:
			parseCGI(it, config);
			break;
		case T_LIMIT_EXCEPT:
			locParseAllowedMethods(it, config);
			break;
		case T_ALLOW_METHODS:
			locParseAllowedMethods(it, config);
			break;
		case T_RETURN:
			locParseReturnCode(it, config);
			break;
		case T_UPLOAD_PATH:
			locParseUploadPath(it, config);
			break;
		default:
			std::string errMsg = "Error\nInvalid token in location context: " + *it;
			throw ParseErrException(errMsg.c_str());
			break;
	}
}

// --------- SHARED CASES ---------

/// @brief Parses and adds root
/// @exception Empty root, more than one root, missing semicolon, root not absolute
template <typename T>
void    ParseConfig::parseRoot(std::vector<std::string>::iterator &it, T &config)
{
	it++; // Skip root
	if (!config.getRoot().empty()) //+ Check for duplicates
		throw ParseErrException("Error\nMore than one root.");

	if (it == this->_tokens.end() || *it == ";") //+ Check if contains a value
		throw ParseErrException("Error\nEmpty root directive.");

	if ((*it)[0] != '/') //+ Check if root is absolute
		throw ParseErrException("Error\nRoot value not absolute.");
		
	config.setRoot(*it); //+ Add root
	it++; // Skip value

	if (it == this->_tokens.end() || *it != ";") //+ Check for end or semicolon
		throw ParseErrException("Error\nMissing expected semicolon after root.");
	it++; // Skip semicolon
}

/// @brief Parses and adds indexes to the config
/// @exception Empty index, missing semicolon
template <typename T>
void	ParseConfig::parseIndex(std::vector<std::string>::iterator &it, T &config)
{
	std::vector<std::string> newIndexes;

	it++; // Skip index
	while (it != this->_tokens.end() && *it != ";") //+ Store in vector
	{
		newIndexes.push_back(*it);
		it++; 
	}

	if (it == this->_tokens.end()) //+ Check for end and semicolon
		throw ParseErrException("Error\nMissing expected semicolon after index.");
		
	if (newIndexes.empty()) //+ Check index not empty
		throw ParseErrException("Error\nEmpty index directive.");
		
	for (std::vector<std::string>::iterator i_it = newIndexes.begin(); i_it != newIndexes.end(); ++i_it)
	{
		config.addIndex(*i_it); //+ Add to GlobalConfig
	}
	it++; //Skip semicolon
}

/// @brief Function to parse and add error pages to the config
/// @exception Missing semicolon, emptz directive, error non number, missing code or path
template <typename T>
void	ParseConfig::parseErrorPages(std::vector<std::string>::iterator &it, T &config)
{
	std::vector<std::string>			temp;
	std::vector<std::string>::iterator	t_it;

	it++; // Skip err_page
	while (it != this->_tokens.end() && *it != ";") //+ Populate vector
	{
		temp.push_back(*it);
		it++;
	}
	if (it == this->_tokens.end()) //+ Check semicolon or end
		throw ParseErrException("Error\nMissing expected semicolon after error_page.");
	it++; // Skip semicolon
	if (temp.empty()) //+ Check not empty
		throw ParseErrException("Error\nEmpty error_page directive.");
	std::string path = temp.back(); //+ Store last one (always the path)
	t_it = temp.begin();
	if (temp.size() < 2) //+ Check at least 2, need HTTP error code + Value (path to page)
		throw ParseErrException("Error\nMissing path or error code in error_pages.");
	while (t_it != temp.end() - 1)
	{
		if ((*t_it).find_first_not_of("0123456789") != std::string::npos) //+ Check all error codes until prev to last one
			throw ParseErrException("Error\nWrong error code in error_page non numeric.");
		int	code = std::atoi((*t_it).c_str());
		if (code < 400 || code > 599 )
			throw ParseErrException("Error\nWrong error-code in error_page - out of bounds (400 - 599).");
		config.addErrorPage(code, path); //+ Add each error code with the path
		t_it++;
	}
}

/// @brief Function to parse and add autoindex to the config
/// @exception Missing semicolon, empty value, accepts onlz "yes" or "no"
template <typename T>
void	ParseConfig::parseAutoindex(std::vector<std::string>::iterator &it, T &config)
{
	it++; // Skip autoindex
	if (it == this->_tokens.end() || *it == ";") //+ Check if not empty
		throw ParseErrException("Error\nWrong autoindex directive.");

	if ((*it).compare("yes")) //+ Validate yes
		config.setAutoindex(true);
	else if ((*it).compare("no")) //+ Validate no
		config.setAutoindex(false);
	else
		throw ParseErrException("Error\nWrong autoindex directive.");

	it++; // Skip value
	if (it == this->_tokens.end() || *it != ";")
		throw ParseErrException("Error\nMissing expected semicolon after autoindex.");
	it++; // Skip semicolon
}

/// @brief Function to parse and add client_max_body_size to the config
/// @exception Missing semicolon, empty value, NaN
template <typename T>
void	ParseConfig::parseMaxBody(std::vector<std::string>::iterator &it, T &config)
{
	it++; // Skip client_max_body_size
	if (it == this->_tokens.end() || *it == ";") //+ Check if empty
		throw ParseErrException("Error\nWrong max_body directive.");

	if ((*it).find_first_not_of("0123456789") != std::string::npos) //+ Check if numeric
		throw ParseErrException("Error\nMax body value needs to be a number.");

	char *end;
	long code = std::strtol((*it).c_str(), &end, 10);
	if (code < 0 || code > MAX_BODY)
		throw ParseErrException("Error\nMax body value out of range.");
	config.setClientMaxBodySize(code);

	it++; // Skip value
	if (it == this->_tokens.end() || *it != ";") //+ Find semicolon
		throw ParseErrException("Error\nMissing expected semicolon after max-body.");
	it++; // Skip semicolon
}

/// @brief Function to parse and add CGI to the config
/// @exception Missing semicolon, empty value, wrong format, more than one
template <typename T>
void    ParseConfig::parseCGI(std::vector<std::string>::iterator &it, T &config)
{
    it++; // Skip cgi_handler
    if (it == this->_tokens.end() || *it == ";" || *it == "}") //+ Check empty extension
        throw ParseErrException("Error\nEmpty cgi directive.");
	
    if ((*it)[0] != '.') //+ Check extension to start with .
        throw ParseErrException("Error\nCGI extension must start with '.'");
    std::string ext = *it;

    it++; // Go to path

    if (it == this->_tokens.end() || *it == ";" || *it == "}") //+ Check empty path
        throw ParseErrException("Error\nMissing cgi path.");

    if ((*it)[0] != '/') //+ Check valid dir
        throw ParseErrException("Error\nCGI path must be absolute.");
    std::string path = *it;
    
	it++;
    if (it == this->_tokens.end() || *it == "}") //+ Find semicolon or end
        throw ParseErrException("Error\nMissing semicolon after cgi.");
    if (*it != ";")
        throw ParseErrException("Error\nUnexpected token after cgi.");

    if (!config.getCgiHandler().first.empty() && !config.getCgiHandler().second.empty())
        throw ParseErrException("Error\nMore than one cgi directive.");
    config.setCgiHandler(std::make_pair(ext, path));
    it++;
}

// --------- SERVER CASES ---------

/// @brief Parses and adds the listen port for that server
/// @exception Empty listen, Invalid format, Missing semicolon
void ParseConfig::servParseListen(std::vector<std::string>::iterator &it, ServerConfig &config)
{
	it++; // Skip listen
	if (it == this->_tokens.end() || *it == ";") //+ Check if empty
		throw ParseErrException("Error\nEmpty listen directive.");

	if (!validListen(*it)) //+ Validation helper
		throw ParseErrException("Error\nInvalid listen format.");

	size_t colon = (*it).find(':');
	if (colon == std::string::npos)
		config.addListen("0.0.0.0", *it); //+ Add default
	else
		config.addListen((*it).substr(0, colon), (*it).substr(colon + 1)); //+ Split

	it++; // Skip ip
	if (it == this->_tokens.end() || *it != ";") //+ Check for semicolon
		throw ParseErrException("Error\nMissing semicolon after listen.");
	it++; // Skip semicolon
}

/// @brief Function to parse and add server_name to the config
/// @exception Missing semicolon, invalid names, name too long
void ParseConfig::servParseServerName(std::vector<std::string>::iterator &it, ServerConfig &config)
{
	it++; // Skip server_name

	if (it != this->_tokens.end() && *it == ";") //+ If emopty, accept all request
	{
		it++;
		return;
	}

	while (it != this->_tokens.end() && *it != ";" && *it != "}")
	{
		if ((*it).size() > 253) //+ Check length
			throw ParseErrException("Error\nServer name too long.");
		for (size_t i = 0; i < (*it).size(); i++)
		{
			if (!isalnum((*it)[i]) && (*it)[i] != '-' && (*it)[i] != '.' && (*it)[i] != '_') //+ Valid name
				throw ParseErrException("Error\nInvalid server_name value.");
		}
		config.addServerName(*it);
		it++;
	}

	if (it == this->_tokens.end() || *it == "}" || *it != ";") //+ Find semicolon
		throw ParseErrException("Error\nMissing semicolon after server_name.");
	it++; // Skip semicolon
}

// --------- LOCATION CASES ---------

/// @brief Function to parse and add allowed methods to the config
/// @exception Missing semicolon, invalid names
void	ParseConfig::locParseAllowedMethods(std::vector<std::string>::iterator &it, LocationConfig &config)
{
	it++; // Skip allowed_methods
	if (it == this->_tokens.end() || *it == ";") //+ Check if empty
		throw ParseErrException("Error\nEmpty allowed methods directive.");
	
	std::vector<std::string>	methods;
	while (it != this->_tokens.end() && *it != ";" && *it != "}")
	{
		if (!validMethod(*it)) //+ Validate method GET, POST, DELETE
			throw ParseErrException("Error\nNot allowed method.");
		methods.push_back(*it);
		it++; // Next method
	}
	config.setAllowMethods(methods);

	if (it == this->_tokens.end() || *it == "}" || *it != ";") //+ Find semicolon
		throw ParseErrException("Error\nMissing semicolon after server_name.");
	it++; // Skip semicolon
}

void	ParseConfig::locParseReturnCode(std::vector<std::string>::iterator &it, LocationConfig &config)
{
	it++; // Skip return
	
	if (it == this->_tokens.end() || *it == ";") //+ Check if empty
		throw ParseErrException("Error\nEmpty return directive.");

	if ((*it).find_first_not_of("0123456789") != std::string::npos) //+ Check if numeric
		throw ParseErrException("Error\nReturn code must be numeric.");
	
	int code = std::atoi((*it).c_str());
	if (code != 301 && code != 302 && code != 307 && code != 308) //+ Check valid code
		throw ParseErrException("Error\nInvalid return code (must be 301, 302, 307, or 308).");
	
	it++; // Move to URL
	
	if (it == this->_tokens.end() || *it == ";") //+ Check if no URL
		throw ParseErrException("Error\nMissing URL in return directive.");
		
	std::string url = *it;
	config.setReturn(code, url);
	
	it++;
	if (it == this->_tokens.end() || *it != ";") //+ Check Semicolon
		throw ParseErrException("Error\nMissing semicolon after return directive.");
	
	it++; // Skip semicolon
}

void	ParseConfig::locParseUploadPath(std::vector<std::string>::iterator &it, LocationConfig &config)
{
	it++; // Skip upload_path
	
	if (it == this->_tokens.end() || *it == ";")
		throw ParseErrException("Error\nEmpty upload_path directive.");

	if ((*it)[0] != '/') //+ Valid path
		throw ParseErrException("Error\nUpload path must be absolute (start with '/').");
		
	config.setUploadPath(*it);
	
	it++;
	if (it == this->_tokens.end() || *it != ";") //+ Check semicolon
		throw ParseErrException("Error\nMissing semicolon after upload_path.");
		
	it++; // Skip semicolon
}

// --------- COMPILER STUFF ---------

template void ParseConfig::parseRoot<GlobalConfig>(std::vector<std::string>::iterator &, GlobalConfig &);
template void ParseConfig::parseRoot<ServerConfig>(std::vector<std::string>::iterator &, ServerConfig &);
template void ParseConfig::parseRoot<LocationConfig>(std::vector<std::string>::iterator &, LocationConfig &);