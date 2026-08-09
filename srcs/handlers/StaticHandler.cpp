#include "../../includes/StaticHandler.hpp"
#include "../../includes/ParseConfig.hpp"
#include <cctype>

/// @brief  easier to use a struct, to separate the name from the data
struct MultipartPart
{
	std::string filename;
	std::string data;
};

/// @brief Convert an ASCII string to lower case for case-insensitive HTTP comparisons.
/// @return A lower-case copy of value.
static std::string toLowerAscii(const std::string &value)
{
	std::string result = value;
	for (size_t i = 0; i < result.size(); ++i)
		result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
	return result;
}

/// @brief Remove ASCII whitespace from string.
/// @return A copy of value without whitespace.
static std::string trim(const std::string &value)
{
	size_t begin = 0;
	size_t end = value.size();
	while (begin < end && std::isspace(static_cast<unsigned char>(value[begin])))
		++begin;
	while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])))
		--end;
	return value.substr(begin, end - begin);
}

/// @brief Extract and validate the boundary parameter of a multipart Content-Type header.
/// @param content_type Value of the Content-Type HTTP header.
/// @return The multipart boundary without optional surrounding quotes.
/// @exception HttpException 400 if the media type or boundary is invalid.
static std::string getMultipartBoundary(const std::string &content_type)
{
	const std::string lower_content_type = toLowerAscii(content_type);
	const size_t media_type_end = lower_content_type.find(';');
	if (trim(lower_content_type.substr(0, media_type_end)) != "multipart/form-data")
		throw HttpException(400, "Bad Request");

	size_t parameter = media_type_end;
	while (parameter != std::string::npos && parameter < content_type.size())
	{
		parameter = content_type.find_first_not_of("; \t", parameter);
		if (parameter == std::string::npos)
			break;
		size_t parameter_end = content_type.find(';', parameter);
		std::string current = trim(content_type.substr(parameter, parameter_end - parameter));
		size_t equal = current.find('=');
		if (equal != std::string::npos && toLowerAscii(trim(current.substr(0, equal))) == "boundary")
		{
			std::string boundary = trim(current.substr(equal + 1));
			if (boundary.size() >= 2 && boundary[0] == '"' && boundary[boundary.size() - 1] == '"')
				boundary = boundary.substr(1, boundary.size() - 2);
			if (boundary.empty() || boundary.size() > 70 || boundary.find_first_of("\r\n") != std::string::npos)
				throw HttpException(400, "Bad Request");
			return boundary;
		}
		parameter = parameter_end;
	}
	throw HttpException(400, "Bad Request");
}

/// @brief Extract the optional filename parameter from a Content-Disposition header.
/// @param disposition Value of a multipart part's Content-Disposition header.
/// @return The filename, or an empty string when the part is not a file.
/// @exception HttpException 400 if the disposition type is invalid.
static std::string getFilenameFromDisposition(const std::string &disposition)
{
	size_t parameter = disposition.find(';');
	if (toLowerAscii(trim(disposition.substr(0, parameter))) != "form-data")
		throw HttpException(400, "Bad Request");
	while (parameter != std::string::npos && parameter < disposition.size())
	{
		parameter = disposition.find_first_not_of("; \t", parameter);
		if (parameter == std::string::npos)
			break;
		size_t parameter_end = disposition.find(';', parameter);
		std::string current = trim(disposition.substr(parameter, parameter_end - parameter));
		size_t equal = current.find('=');
		if (equal != std::string::npos && toLowerAscii(trim(current.substr(0, equal))) == "filename")
		{
			std::string filename = trim(current.substr(equal + 1));
			if (filename.size() >= 2 && filename[0] == '"' && filename[filename.size() - 1] == '"')
				filename = filename.substr(1, filename.size() - 2);
			return filename;
		}
		parameter = parameter_end;
	}
	return "";
}

/// @brief Validate a client-provided filename and reduce it to a safe basename.
/// @param filename Filename extracted from Content-Disposition.
/// @return A safe filename that cannot select another directory.
/// @exception HttpException 400 if the filename is unsafe or invalid.
static std::string sanitizeUploadFilename(const std::string &filename)
{
	std::string safe_name = filename;
	size_t separator = safe_name.find_last_of("/\\");
	if (separator != std::string::npos)
		safe_name = safe_name.substr(separator + 1);
	if (safe_name.empty() || safe_name == "." || safe_name == ".." || safe_name.find("..") != std::string::npos)
		throw HttpException(400, "Bad Request");
	for (size_t i = 0; i < safe_name.size(); ++i)
	{
		if (safe_name[i] == '\0' || static_cast<unsigned char>(safe_name[i]) < 32)
			throw HttpException(400, "Bad Request");
	}
	return safe_name;
}

/// @brief Parse file parts from a complete multipart/form-data request body.
/// @param body Complete HTTP request body.
/// @param boundary Boundary extracted from the Content-Type header.
/// @return All file parts, excluding regular form fields without a filename.
/// @exception HttpException 400 if the multipart body format is malformed.
static std::vector<MultipartPart> parseMultipartBody(const std::string &body, const std::string &boundary)
{
	const std::string delimiter = "--" + boundary;
	if (body.compare(0, delimiter.size(), delimiter) != 0 || body.size() < delimiter.size() + 2)
		throw HttpException(400, "Bad Request");

	std::vector<MultipartPart> parts;
	size_t position = delimiter.size();
	if (body.compare(position, 2, "--") == 0)
		return parts;
	if (body.compare(position, 2, "\r\n") != 0)
		throw HttpException(400, "Bad Request");
	position += 2;

	while (position < body.size())
	{
		size_t headers_end = body.find("\r\n\r\n", position);
		if (headers_end == std::string::npos)
			throw HttpException(400, "Bad Request");

		std::string disposition;
		std::istringstream header_stream(body.substr(position, headers_end - position));
		std::string header;
		while (std::getline(header_stream, header))
		{
			if (!header.empty() && header[header.size() - 1] == '\r')
				header.erase(header.size() - 1);
			size_t colon = header.find(':');
			if (colon == std::string::npos)
				throw HttpException(400, "Bad Request");
			if (toLowerAscii(trim(header.substr(0, colon))) == "content-disposition")
				disposition = trim(header.substr(colon + 1));
		}
		if (disposition.empty())
			throw HttpException(400, "Bad Request");

		position = headers_end + 4;
		size_t next_boundary = body.find("\r\n" + delimiter, position);
		if (next_boundary == std::string::npos)
			throw HttpException(400, "Bad Request");

		std::string filename = getFilenameFromDisposition(disposition);
		if (!filename.empty())
		{
			MultipartPart part;
			part.filename = sanitizeUploadFilename(filename);
			part.data = body.substr(position, next_boundary - position);
			parts.push_back(part);
		}

		position = next_boundary + 2 + delimiter.size();
		if (body.compare(position, 2, "--") == 0)
		{
			position += 2;
			if (position != body.size() && body.compare(position, 2, "\r\n") != 0)
				throw HttpException(400, "Bad Request");
			return parts;
		}
		if (body.compare(position, 2, "\r\n") != 0)
			throw HttpException(400, "Bad Request");
		position += 2;
	}
	throw HttpException(400, "Bad Request");
}

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
	if (isDirectoryPath(this->_path))
	{
		this->GetDirectory();
		return;
	}
	if (!isValidFile(this->_path, F_OK))
		throw HttpException(404, "Not Found");
	this->GetFile();

		
}

/// @brief try to post the asked file and build the 201 appropriate response
/// @exception 403, 404, 507, 500
void	StaticHandler::Post()
{
	const std::map<std::string, std::string> headers = this->getRequestHandler().getHeaders();
	std::map<std::string, std::string>::const_iterator ct_it = headers.find("content-type");
	std::string body_to_write = this->getRequestHandler().getBody();

	if (ct_it != headers.end() && ct_it->second.find("multipart/form-data") != std::string::npos)
	{
		const std::string &upload_root = this->_location.getUploadPath();
		if (upload_root.empty())
			throw HttpException(403, "Forbidden");

		std::vector<MultipartPart> parts = parseMultipartBody(body_to_write, getMultipartBoundary(ct_it->second));
		if (parts.empty())
			throw HttpException(400, "Bad Request");

		for (size_t i = 0; i < parts.size(); ++i)
		{
			std::ofstream out(joinPath("." + upload_root, parts[i].filename).c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
			if (!out.is_open())
				throw HttpException(403, "Forbidden");
			out.write(parts[i].data.c_str(), static_cast<std::streamsize>(parts[i].data.size()));
			if (!out.good())
				throw HttpException(500, "Internal Server Error in Post");
		}

		this->_status_code = 201;
		this->_response_body = "Created\n";
		this->_content_type = "text/plain";
		return;
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
	if (access(this->_path.c_str(), R_OK) != 0)
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

	parseMultipartBody(this->getRequestHandler().getBody(), getMultipartBoundary(ct_it->second));

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