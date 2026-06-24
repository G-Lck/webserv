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