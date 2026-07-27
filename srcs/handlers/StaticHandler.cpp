#include "../../includes/StaticHandler.hpp"

StaticHandler::StaticHandler() {}

StaticHandler::StaticHandler(const StaticHandler& other) { *this = other; }

StaticHandler& StaticHandler::operator=(const StaticHandler& other) {
	if (this != &other) {
		Handler::operator=(other);
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
}

/// @brief try to delete the asked file and build the 204 appropriate response
/// @exception 403, 404, 500
void	StaticHandler::Delete()
{
	CanDelete(); // if not, an exception is throw and we stop there
	//delete and build the 204 response

}

/// @brief when it's a directory, forbidden if we can't open it, create an html or give the index depending
/// on the aut-index (not sure about that)
/// @exception 403, 500
void	StaticHandler::GetDirectory()
{

}

/// @brief when it's a file, if we can open it, construct the 200 response
/// @exception 403, 500
void	StaticHandler::GetFile()
{

}

/// @brief  if content-type is mujltipart-data we have a parsing to do
void	StaticHandler::parseMultipartData()
{

}

/// @brief check if the fils exist, if we can delete it or if something went wrong
/// @exception 404, 403, 500
void	StaticHandler::CanDelete()
{

}