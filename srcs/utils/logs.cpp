#include "../../includes/WebServ.hpp"

void	logError( const std::string errMsg, int flag )
{
	if (flag == ACCESS)
	{
		std::ofstream outfile_access("./log/access.log", std::ios::app);
		outfile_access << errMsg;
	}
	else
	{
		switch(flag)
		{
			case ERROR_INFO:
			{
				std::ofstream outfile_info("./log/error_info.log", std::ios::app);
				outfile_info << errMsg;
				break ;
			}
			case ERROR_WARNING:
			{
				std::ofstream outfile_info("./log/error_info.log", std::ios::app);
				outfile_info << errMsg;
				std::ofstream outfile_warning("./log/error_warning.log", std::ios::app);
				outfile_warning << errMsg;
				break ;
			}
			default:
				break;
		}

	}
}