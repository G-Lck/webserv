#include "../../includes/ParseConfig.hpp"

// ------ Orthodox ------

ParseConfig::ParseConfig() {}
ParseConfig::ParseConfig(const ParseConfig &other) {}
ParseConfig &ParseConfig::operator=(const ParseConfig &other) {}
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
	std::string	dir = dir_c;
	std::ifstream	givenFile(dir);
	if (!givenFile.is_open())
		throw ParseErrException("Error\nWrong File");

	std::string	text;
	std::string	buff;
	getline(givenFile, buff);
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
	if (!validBrackets(text))
		throw ParseErrException("Error\nUnclosed Brackets {}");
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
    if (token == "root") return G_ROOT;
    if (token == "index") return G_INDEX;
    if (token == "error_page") return G_ERROR_PAGE;
    if (token == "autoindex") return G_AUTOINDEX;
    if (token == "client_max_body_size") return G_MAX_BODY;
    if (token == "server") return G_SERVER;
    return G_UNKNOWN;
}

int ParseConfig::getServerType(std::string& token)
{
    if (token == "listen") return S_LISTEN;
    if (token == "server_name") return S_SERVER_NAME;
    if (token == "root") return S_ROOT;
    if (token == "error_page") return S_ERROR_PAGE;
    if (token == "autoindex") return S_AUTOINDEX;
    if (token == "client_max_body_size") return S_MAX_BODY;
    if (token == "cgi_handler") return S_CGI;
    if (token == "location") return S_LOCATION;
    return S_UNKNOWN;
}

int ParseConfig::getLocationType(std::string& token)
{
    if (token == "root") return L_ROOT;
    if (token == "index") return L_INDEX;
    if (token == "error_page") return L_ERROR_PAGE;
    if (token == "autoindex") return L_AUTOINDEX;
    if (token == "client_max_body_size") return L_MAX_BODY;
    if (token == "cgi_handler") return L_CGI;
    if (token == "limit_except") return L_LIMIT_EXCEPT;
    if (token == "return") return L_RETURN;
    return L_UNKNOWN;
}

// --------- SWITCHES ---------

void ParseConfig::parseGlobalToken(std::vector<std::string>::iterator &it, GlobalConfig &config)
{
	int type = getGlobalType(*it);

	switch (type)
	{
		case G_ROOT:
			rootCase(it, config);
			break;
		case G_INDEX:
			indexCase(it, config);
			break;
		case G_ERROR_PAGE:
			errorPagesCase(it, config);
			break;
		case G_AUTOINDEX:
			autoindexCase(it, config);
			break;
		case G_MAX_BODY:
			maxBodyCase(it, config);
			break;
		default:
			std::string	errMsg = "Error\nInvalid token in global context: " + *it;
			throw ParseErrException(errMsg.c_str());
			break;
    }
}

void ParseConfig::parseServerToken(std::vector<std::string>::iterator &it, ServerConfig &config)
{
    int type = getServerType(*it);

    switch (type)
    {
		case S_LISTEN:
			servListenCase(it, config);
			break;
		case S_SERVER_NAME:
			servServerNameCase(it, config);
			break;
		case S_ROOT:
			servRootCase(it, config);
			break;
		case S_ERROR_PAGE:
			servErrorPagesCase(it, config);
			break;
		case S_AUTOINDEX:
			servAutoindexCase(it, config);
			break;
		case S_MAX_BODY:
			servMaxBodyCase(it, config);
			break;
		case S_CGI:
			servCgiCase(it, config);
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
        case L_ROOT:
            // handle root
            break;
        case L_INDEX:
            // handle index
            break;
        case L_ERROR_PAGE:
            // handle error_page
            break;
        case L_AUTOINDEX:
            // handle autoindex
            break;
        case L_MAX_BODY:
            // handle client_max_body_size
            break;
        case L_CGI:
            // handle cgi_handler
            break;
        case L_LIMIT_EXCEPT:
            // handle limit_except
            break;
        case L_RETURN:
            // handle return
            break;
        default:
            std::string errMsg = "Error\nInvalid token in location context: " + *it;
            throw ParseErrException(errMsg.c_str());
            break;
    }
}

// --------- CONGIF CASES ---------

/// @brief Parses and adds root to the config
/// @exception Empty root, more than one root, missing semicolon
void ParseConfig::rootCase(std::vector<std::string>::iterator &it, GlobalConfig &config)
{
	it++; // Skip root
	if (!config.getRoot().empty())
		throw ParseErrException("Error\nMore than one root.");

	if (it == this->_tokens.end() || *it == ";")
		throw ParseErrException("Error\nEmpty root directive.");
		
	config.setRoot(*it);
	it++; // Skip value

	if (it == this->_tokens.end() || *it != ";")
		throw ParseErrException("Error\nMissing expected semicolon after root.");
	it++; // Skip semicolon
}

/// @brief Parses and adds indexes to the config
/// @exception Empty index, missing semicolon
void ParseConfig::indexCase(std::vector<std::string>::iterator &it, GlobalConfig &config)
{
	std::vector<std::string> newIndexes;

	it++; // Skip index
	while (it != this->_tokens.end() && *it != ";")
	{
		newIndexes.push_back(*it);
		it++; 
	}

	if (it == this->_tokens.end())
		throw ParseErrException("Error\nMissing expected semicolon after index.");
		
	if (newIndexes.empty())
		throw ParseErrException("Error\nEmpty index directive.");
		
	for (std::vector<std::string>::iterator i_it = newIndexes.begin(); i_it != newIndexes.end(); ++i_it)
	{
		config.addIndex(*i_it);
	}
	it++; //Skip semicolon
}

/// @brief Function to parse and add error pages to the config
/// @exception Missing semicolon, emptz directive, error non number, missing code or path
void ParseConfig::errorPagesCase(std::vector<std::string>::iterator &it, GlobalConfig &config)
{
	std::vector<std::string>			temp;
	std::vector<std::string>::iterator	t_it;

	it++; // Skip err_page
	while (it != this->_tokens.end() && *it != ";")
	{
		temp.push_back(*it);
		it++;
	}
	if (it == this->_tokens.end())
		throw ParseErrException("Error\nMissing expected semicolon after error_page.");
	it++; // Skip semicolon
	if (temp.empty())
		throw ParseErrException("Error\nEmpty error_page directive.");
	std::string path = temp.back();
	t_it = temp.begin();
	if (temp.size() < 2)
		throw ParseErrException("Error\nMissing path or error code in error_page.");
	while (t_it != temp.end() - 1)
	{
		if ((*t_it).find_first_not_of("0123456789") != std::string::npos)
			throw ParseErrException("Error\nWrong error-code in error_page directive.");
		int	code = std::atoi((*t_it).c_str());
		config.addErrorPage(code, path);
		t_it++;
	}
}

/// @brief Function to parse and add autoindex to the config
/// @exception Missing semicolon, empty value, accepts onlz "yes" or "no"
void ParseConfig::autoindexCase(std::vector<std::string>::iterator &it, GlobalConfig &config)
{
	it++; // Skip autoindex
	if (it == this->_tokens.end() || *it == ";")
		throw ParseErrException("Error\nWrong autoindex directive.");
	if (!(*it).compare("yes"))
		config.setAutoindex(true);
	else if (!(*it).compare("no"))
		config.setAutoindex(false);
	else
		throw ParseErrException("Error\nWrong autoindex directive.");
	it++; // Skip value
	if (it == this->_tokens.end() || *it != ";")
		throw ParseErrException("Error\nMissing expected semicolon after autoindex.");
	it++; // Skip semicolon
}

/// @brief Function to parse and add autoindex to the config
/// @exception Missing semicolon, empty value, accepts onlz "yes" or "no"
void ParseConfig::maxBodyCase(std::vector<std::string>::iterator &it, GlobalConfig &config)
{
	it++; // Skip autoindex
	if (it == this->_tokens.end() || *it == ";")
		throw ParseErrException("Error\nWrong max_body directive.");
	if ((*it).find_first_not_of("0123456789") != std::string::npos)
		throw ParseErrException("Error\nMax body value needs to be a number.");
	int	code = std::atoi((*it).c_str());
	config.setClientMaxBodySize(code);
	it++; // Skip value
	if (it == this->_tokens.end() || *it != ";")
		throw ParseErrException("Error\nMissing expected semicolon after max-body.");
	it++; // Skip semicolon
}

// --------- SERVER CASES ---------

/// @brief Parses and adds the listen port for that server
/// @exception Empty listen, Invalid format, Missing semicolon
void ParseConfig::servListenCase(std::vector<std::string>::iterator &it, ServerConfig &config)
{
	it++; // Skip listen
	if (!config.getListen().first.empty() && !config.getListen().second.empty())
		throw ParseErrException("Error\nMore than one listen.");
	if (it == this->_tokens.end() || *it == ";")
		throw ParseErrException("Error\nEmpty listen directive.");
	if (!validListen(*it))
		throw ParseErrException("Error\nInvalid listen format.");
	size_t colon = (*it).find(':');
	if (colon == std::string::npos)
		config.addListen("0.0.0.0", *it);
	else
		config.addListen((*it).substr(0, colon), (*it).substr(colon + 1));
	it++; // Skip ip
	if (it == this->_tokens.end() || *it != ";")
		throw ParseErrException("Error\nMissing semicolon after listen.");
	it++; // Skip semicolon
}

/// @brief Parses and adds root to the config
/// @exception Empty root, more than one root, missing semicolon
void ParseConfig::servRootCase(std::vector<std::string>::iterator &it, ServerConfig &config)
{
	it++; // Skip root
	if (!config.getRoot().empty())
		throw ParseErrException("Error\nMore than one root.");

	if (it == this->_tokens.end() || *it == ";")
		throw ParseErrException("Error\nEmpty root directive.");
		
	config.setRoot(*it);
	it++; // Skip value

	if (it == this->_tokens.end() || *it != ";")
		throw ParseErrException("Error\nMissing expected semicolon after root.");
	it++; // Skip semicolon
}

/// @brief Function to parse and add error pages to the config
/// @exception Missing semicolon, emptz directive, error non number, missing code or path
void ParseConfig::servErrorPagesCase(std::vector<std::string>::iterator &it, ServerConfig &config)
{
	std::vector<std::string>			temp;
	std::vector<std::string>::iterator	t_it;

	it++; // Skip err_page
	while (it != this->_tokens.end() && *it != ";")
	{
		temp.push_back(*it);
		it++;
	}
	if (it != this->_tokens.end() && *it != ";" && *it != "}")
		throw ParseErrException("Error\nMissing expected semicolon after error_page.");
	it++; // Skip semicolon
	if (temp.empty())
		throw ParseErrException("Error\nEmpty error_page directive.");
	std::string path = temp.back();
	t_it = temp.begin();
	if (temp.size() < 2)
		throw ParseErrException("Error\nMissing path or error code in error_page.");
	while (t_it != temp.end() - 1)
	{
		if ((*t_it).find_first_not_of("0123456789") != std::string::npos)
			throw ParseErrException("Error\nWrong error-code in error_page directive.");
		int	code = std::atoi((*t_it).c_str());
		config.addErrorPage(code, path);
		t_it++;
	}
}

/// @brief Function to parse and add autoindex to the config
/// @exception Missing semicolon, empty value, accepts onlz "yes" or "no"
void ParseConfig::servAutoindexCase(std::vector<std::string>::iterator &it, ServerConfig &config)
{
	it++; // Skip autoindex
	if (it == this->_tokens.end() || *it == ";")
		throw ParseErrException("Error\nWrong autoindex directive.");
	if (!(*it).compare("yes"))
		config.setAutoindex(true);
	else if (!(*it).compare("no"))
		config.setAutoindex(false);
	else
		throw ParseErrException("Error\nWrong autoindex directive.");
	it++; // Skip value
	if (it == this->_tokens.end() || *it != ";")
		throw ParseErrException("Error\nMissing expected semicolon after autoindex.");
	it++; // Skip semicolon
}

/// @brief Function to parse and add autoindex to the config
/// @exception Missing semicolon, empty value, accepts onlz "yes" or "no"
void ParseConfig::servMaxBodyCase(std::vector<std::string>::iterator &it, ServerConfig &config)
{
	it++; // Skip autoindex
	if (it == this->_tokens.end() || *it == ";")
		throw ParseErrException("Error\nWrong max_body directive.");
	if ((*it).find_first_not_of("0123456789") != std::string::npos)
		throw ParseErrException("Error\nMax body value needs to be a number.");
	int	code = std::atoi((*it).c_str());
	config.setClientMaxBodySize(code);
	it++; // Skip value
	if (it == this->_tokens.end() || *it != ";" || *it == "}")
		throw ParseErrException("Error\nMissing expected semicolon after autoindex.");
	it++; // Skip semicolon
}

/// @brief Function to parse and add autoindex to the config
/// @exception Missing semicolon, invalid names, emtpy
void ParseConfig::servServerNameCase(std::vector<std::string>::iterator &it, ServerConfig &config)
{
    it++;
    if (it == this->_tokens.end() || *it == ";")
        throw ParseErrException("Error\nEmpty server_name directive.");
    while (it != this->_tokens.end() && *it != ";" && *it != "}")
    {
        for (size_t i = 0; i < (*it).size(); i++)
            if (!isalnum((*it)[i]) && (*it)[i] != '-' && (*it)[i] != '.')
                throw ParseErrException("Error\nInvalid server_name value.");
        config.addServerName(*it);
        it++;
    }
    if (it == this->_tokens.end() || *it == "}" || *it != ";")
        throw ParseErrException("Error\nMissing semicolon after server_name.");
    it++;
}

/// @brief Function to parse and add CGI to the config
/// @exception Missing semicolon, empty value, wrong format, duplicates
void ParseConfig::servCgiCase(std::vector<std::string>::iterator &it, ServerConfig &config)
{
    it++;
    if (it == this->_tokens.end() || *it == ";" || *it == "}")
        throw ParseErrException("Error\nEmpty cgi directive.");
    if ((*it)[0] != '.')
        throw ParseErrException("Error\nCGI extension must start with '.'");
    std::string ext = *it;
    it++;
    if (it == this->_tokens.end() || *it == ";" || *it == "}")
        throw ParseErrException("Error\nMissing cgi path.");
    if ((*it)[0] != '/')
        throw ParseErrException("Error\nCGI path must be absolute.");
    std::string path = *it;
    it++;
    if (it == this->_tokens.end() || *it == "}")
        throw ParseErrException("Error\nMissing semicolon after cgi.");
    if (*it != ";")
        throw ParseErrException("Error\nUnexpected token after cgi.");
    if (!config.getCgiHandler().empty())
        throw ParseErrException("Error\nDuplicate cgi directive.");
    config.setCgiHandler(ext + " " + path);
    it++;
}


// --------- LOCATION CASES ---------