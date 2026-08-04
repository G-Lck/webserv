#include "../../includes/StaticHandler.hpp"
#include "../../includes/ParseConfig.hpp"

/// @brief  check if the path exist and return true or false if it's a dir or not

static bool isDirectoryPath(const std::string &path)
{
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;
	return S_ISDIR(st.st_mode);
}

/// @brief get the content of the file, from 
/// @param path 
/// @return 
static std::string getFileContentOrThrow(const std::string &path)
{
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		throw HttpException(403, "Forbidden");
	std::ostringstream oss;
	oss << file.rdbuf();
	if (file.bad())
		throw HttpException(500, "Bad file content");
	return oss.str();
}

/// @brief try to guess the type of the file by the end, only html, css,
/// javascript, text, png, jpeg are allowed
/// @return 
static std::string guessContentType(const std::string &path)
{
	if (path.size() >= 5 && path.substr(path.size() - 5) == ".html")
		return "text/html";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".css")
		return "text/css";
	if (path.size() >= 3 && path.substr(path.size() - 3) == ".js")
		return "application/javascript";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".txt")
		return "text/plain";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")
		return "image/png";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg")
		return "image/jpeg";
	if (path.size() >= 5 && path.substr(path.size() - 5) == ".jpeg")
		return "image/jpeg";
	return "application/bin";
}

/// @brief if we can open the directory, we build an html auto index
/// @return this html as a string
static std::string buildAutoIndexHtml(const std::string &path)
{
	DIR *dir = opendir(path.c_str());
	if (!dir)
		throw HttpException(403, "Forbidden");

	std::ostringstream html;
	html << "<html><body><h1>Index of " << path << "</h1><ul>";
	for (dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir))
	{
		std::string name = entry->d_name;
		html << "<li><a href=\"" << name << "\">" << name << "</a></li>";
	}
	html << "</ul></body></html>";
	closedir(dir);
	return html.str();
}

/// @brief  add the slash if needed
/// @return the constructed string
static std::string joinPath(const std::string &base, const std::string &suffix)
{
	if (base.empty())
		return suffix;
	if (base[base.size() - 1] == '/')
		return base + suffix;
	return base + "/" + suffix;
}

/// @brief find the last word after the last / or return the string if none are there
/// if the string end by / it will return by default x.bin

static std::string extractFilename(const std::string &path)
{
	size_t pos = path.find_last_of('/');
	if (pos == std::string::npos)
		return path;
	if (pos + 1 >= path.size())
		return "x.bin";
	return path.substr(pos + 1);
}

/// @brief if it's a multipart it should begin by -- and end with \r\n--

static std::string extractMultipartPayload(const std::string &body)
{
	size_t headers_end = body.find("\r\n\r\n");
	if (headers_end == std::string::npos)
		throw HttpException(400, "Bad Request");

	size_t payload_start = headers_end + 4;
	size_t last_boundary = body.rfind("\r\n--");
	if (last_boundary == std::string::npos || last_boundary < payload_start)
		throw HttpException(400, "Bad Request");

	return body.substr(payload_start, last_boundary - payload_start);
}

StaticHandler::StaticHandler() : _status_code(200), _response_body(""), _content_type("text/plain") {}

StaticHandler::StaticHandler(const Handler& other) : Handler(other), _status_code(200), _response_body(""), _content_type("text/plain") {}

StaticHandler& StaticHandler::operator=(const StaticHandler& other) {
	if (this != &other) {
		Handler::operator=(other);
		this->_status_code = other._status_code;
		this->_response_body = other._response_body;
		this->_content_type = other._content_type;
	}
	return *this;
}

StaticHandler::~StaticHandler() {}

/// @brief  try to get the asked file and build the 200 appropriate response
/// @exception 403, 404, 500
void	StaticHandler::Get()
{
	//if not found throw 404
	//if directory
	//	GetDirectory()
	//if file
	//	GetFile
	this->_status_code = 200;
	if (!isValidFile(this->_path, F_OK))
		throw HttpException(404, "Not Found");

	if (isDirectoryPath(this->_path))
	{
		this->GetDirectory();
		return;
	}
	this->GetFile();

		
}

/// @brief try to post the asked file and build the 201 appropriate response
/// @exception 403, 404, 507, 500
void	StaticHandler::Post()
{
	// if content-type == multipart-data
	//	this->parseMultipartData();
	// check if we can open the file
	// check if we can write in the file
	// post and build the 201 response
	const std::map<std::string, std::string> headers = this->getRequestHandler().getHeaders();
	std::map<std::string, std::string>::const_iterator ct_it = headers.find("content-type");
	std::string body_to_write = this->getRequestHandler().getBody();

	if (ct_it != headers.end() && ct_it->second.find("multipart/form-data") != std::string::npos)
	{
		this->parseMultipartData();
		body_to_write = extractMultipartPayload(body_to_write);
	}

	std::string target_path = this->_path;
	const std::string &upload_root = this->_location.getUploadPath();
	if (!upload_root.empty())
		target_path = joinPath("." + upload_root, extractFilename(this->getRequestHandler().getPath()));

	std::ofstream out(target_path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out.is_open())
		throw HttpException(403, "Forbidden");
	out.write(body_to_write.c_str(), static_cast<std::streamsize>(body_to_write.size()));
	if (!out.good())
		throw HttpException(500, "Internal Server Error in Post");

	this->_status_code = 201;
	this->_response_body = "Created\n";
	this->_content_type = "text/plain";
}

/// @brief try to delete the asked file and build the 204 appropriate response
/// @exception 403, 404, 500
void	StaticHandler::Delete()
{
	CanDelete(); // if not, an exception is throw and we stop there
	//delete and build the 204 response
	if (unlink(this->_path.c_str()) != 0)
		throw HttpException(500, "Internal Server Error in Delete");
	this->_status_code = 204;
	this->_response_body.clear();
	this->_content_type = "text/plain";

}

/// @brief when it's a directory, forbidden if we can't open it, create an html or give the index depending
/// on the aut-index (not sure about that)
/// @exception 403, 500
void	StaticHandler::GetDirectory()
{
	if (!isValidFile(this->_path, R_OK))
		throw HttpException(403, "Forbidden");

	if (this->_location.getAutoindex())
	{
		this->_response_body = buildAutoIndexHtml(this->_path);
		this->_content_type = "text/html";
		return;
	}

	const std::vector<std::string> &indexes = this->_location.getIndex();
	for (size_t i = 0; i < indexes.size(); ++i)
	{
		std::string index_path = joinPath(this->_path, indexes[i]);
		if (isValidFile(index_path, R_OK) && !isDirectoryPath(index_path))
		{
			this->_response_body = getFileContentOrThrow(index_path);
			this->_content_type = guessContentType(index_path);
			return;
		}
	}
	throw HttpException(403, "Forbidden");

}

/// @brief when it's a file, if we can open it, construct the 200 response
/// @exception 403, 500
void	StaticHandler::GetFile()
{
	if (!isValidFile(this->_path, R_OK))
		throw HttpException(403, "Forbidden");
	this->_response_body = getFileContentOrThrow(this->_path);
	this->_content_type = guessContentType(this->_path);

}

/// @brief  if content-type is mujltipart-data we have a parsing to do
void	StaticHandler::parseMultipartData()
{
	const std::map<std::string, std::string> headers = this->getRequestHandler().getHeaders();
	std::map<std::string, std::string>::const_iterator ct_it = headers.find("content-type");
	if (ct_it == headers.end())
		throw HttpException(400, "Bad Request");

	if (ct_it->second.find("multipart/form-data") == std::string::npos)
		return;

	if (ct_it->second.find("boundary=") == std::string::npos)
		throw HttpException(400, "Bad Request");

	const std::string &body = this->getRequestHandler().getBody();
	if (body.find("\r\n\r\n") == std::string::npos)
		throw HttpException(400, "Bad Request");

}

/// @brief check if the fils exist, if we can delete it or if something went wrong
/// @exception 404, 403, 500
void	StaticHandler::CanDelete()
{
	if (!isValidFile(this->_path, F_OK))
		throw HttpException(404, "Not Found");
	if (isDirectoryPath(this->_path))
		throw HttpException(403, "Forbidden");
	if (!isValidFile(this->_path, W_OK))
		throw HttpException(403, "Forbidden");

}

int	StaticHandler::getStatusCode() const
{
	return this->_status_code;
}

const std::string&	StaticHandler::getResponseBody() const
{
	return this->_response_body;
}

const std::string&	StaticHandler::getContentType() const
{
	return this->_content_type;
}