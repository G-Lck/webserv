#include "../../includes/WebServ.hpp"

bool validBrackets(const std::string &text)
{
	int brackets = 0;
	for (size_t i = 0; i < text.size(); i++)
	{
		if (text[i] == '{') brackets++;
		if (text[i] == '}') brackets--;
		if (brackets < 0)
			return false;
	}
	return brackets == 0;
}

/// @brief Simple boolean function to validate the error page path
/// @param path The string to validate
bool isValidErrorPagePath(const std::string& path)
{
    if (path.empty())
        return false;
    //+ 1. Must start with '/' (absolute path relative to server root)
    if (path[0] != '/')
        return false;
    //+ 2. Must not end with '/' (it must be a file, not a directory)
    if (path[path.length() - 1] == '/')
        return false;
    //+ 3. Must contain a file extension (e.g., .html, .htm)
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == path.length() - 1)
        return false;
    return true;
}

/// @brief Function to validate the format of the IP
/// @param host the string to validate
bool validIP(const std::string &host)
{
    int  dots = 0;
    long num = 0; 
    bool hasDigit = false;

    for (size_t i = 0; i < host.size(); i++)
    {
        if (host[i] == '.')
        {
            if (!hasDigit || num > 255)
                return false;
            dots++;
            num = 0;
            hasDigit = false;
        }
        else if (isdigit(host[i]))
        {
            num = num * 10 + (host[i] - '0');
            if (num > 255) 
                return false; 
            hasDigit = true;
        }
        else
            return false;
    }
    return dots == 3 && hasDigit && num <= 255;
}

/// @brief Checks if the listen argument is valid for ServerConfig
/// @param token the token to validate
bool validListen(const std::string &token)
{
    size_t colon = token.find(':');
    
    if (colon == std::string::npos)
    {
        if (validIP(token)) 
            return true; 
            
        for (size_t i = 0; i < token.size(); i++)
            if (!isdigit(token[i]))
                return false;
        
        if (token.empty()) return false;
        long port = std::atol(token.c_str());
        return (port >= 1 && port <= 65535);
    }

    std::string host = token.substr(0, colon);
    std::string portStr = token.substr(colon + 1);
    
    if (host.empty() || portStr.empty())
        return false;
        
    for (size_t i = 0; i < portStr.size(); i++)
        if (!isdigit(portStr[i]))
            return false;
            
    long port = std::atol(portStr.c_str());
    if (port < 1 || port > 65535)
        return false;

    if (validIP(host)) 
        return true;
        
    for (size_t i = 0; i < host.size(); i++)
        if (!isalnum(host[i]) && host[i] != '-' && host[i] != '.')
            return false;

    return true;
}