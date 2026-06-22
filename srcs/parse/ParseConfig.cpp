#include "../../includes/ParseConfig.hpp"

// ------ Orthodox ------

ParseConfig::ParseConfig() {}
ParseConfig::ParseConfig(const ParseConfig &other) {}
ParseConfig &ParseConfig::operator=(const ParseConfig &other) {}
ParseConfig::~ParseConfig() {}

// ------ Exceptions ------

ParseConfig::ParseErrException::ParseErrException(const char* message) : std::runtime_error(message) {}

// ------ Parsing ------

/// @brief Read the directory and store the content in a string
/// @param dir The directory to read
/// @return The string containing the file text
/// @exception Throws ParseErrExeption in case the file is not found or empty
std::string	ParseConfig::getFileText(const std::string &dir)
{
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
	return (text);
}

/// @brief Tokenizes the text storing it in the vector of string tokens
/// @param fileText The text to tokenize
///	@exception	Throws an exeption for unclosed brackets
void	ParseConfig::tokenizer(std::string fileText, GlobalConfig &config)
{
	size_t	i = 0;
	int		brackets = 0;
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
		if (token[0] == '{')
			brackets++;
		if (token[0] == '}')
			brackets--;
		config.addToken(token);
		i = end;
	}
	if (brackets != 0)
		throw ParseErrException("Error\nUnclosed brackets");
}

void	ParseConfig::parse(const std::string &dir, GlobalConfig &config)
{
	std::vector<std::string>::iterator 	it;
	std::vector<std::string>			tokens;

	tokenizer(getFileText(dir), config);
	tokens = config.getToken();
	it = tokens.begin();

	switch (expression)
	{
	case constant expression:
		/* code */
		break;
	
	default:
		break;
	}
}

