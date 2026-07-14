#include "../../includes/WebServ.hpp"

void	writeLog( const std::string msg, int flag )
{
	if (flag == INFO)
	{
		std::ofstream outfile_info("./log/info.log", std::ios::app);
		outfile_info << msg;
	}
	if (flag == ACCESS)
	{
		std::ofstream outfile_access("./log/access.log", std::ios::app);
		outfile_access << msg;
	}
	if (flag == SERVER_EVENTS)
	{
		std::ofstream outfile_server_events("./log/server_events.log", std::ios::app);
		outfile_server_events << msg;
	}
	else
	{
		switch(flag)
		{
			case ERROR_INFO:
			{
				std::ofstream outfile_info("./log/error_info.log", std::ios::app);
				outfile_info << msg;
				break ;
			}
			case ERROR_WARNING:
			{
				std::ofstream outfile_info("./log/error_info.log", std::ios::app);
				outfile_info << msg;
				std::ofstream outfile_warning("./log/error_warning.log", std::ios::app);
				outfile_warning << msg;
				break ;
			}
			default:
				break;
		}

	}
}