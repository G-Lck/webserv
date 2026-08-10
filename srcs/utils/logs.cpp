#include "../../includes/WebServ.hpp"

/// @brief This function will log: [time] + [your-msg] inside the log file selected tiwh the flag
/// @param msg The message to add
/// @param flag INFO | STATUS_CODE | ACCESS | SERVER_EVENTS | ERROR_INFO | ERROR_WARNING
void	writeLog( const std::string msg, int flag )
{
	if (!g_log)
		return ;
	time_t	now;
	time(&now);
	char *t = ctime(&now);
	t[strlen(t) - 1] = '\0'; //+ remove the new line

	if (flag == INFO)
	{
		std::ofstream outfile_info("./log/info.log", std::ios::app);
		outfile_info << t << ": " << msg << std::endl;
	}
	if (flag == STATUS_CODE)
	{
		std::ofstream outfile_status_code("./log/status_code.log", std::ios::app);
		outfile_status_code << t << ": " << msg << std::endl;
	}
	if (flag == ACCESS)
	{
		std::ofstream outfile_access("./log/access.log", std::ios::app);
		outfile_access << t << ": " << msg << std::endl;
	}
	if (flag == SERVER_EVENTS)
	{
		std::ofstream outfile_server_events("./log/server_events.log", std::ios::app);
		outfile_server_events << t << ": " << msg << std::endl;
	}
	else
	{
		switch(flag)
		{
			case ERROR_INFO:
			{
				std::ofstream outfile_info("./log/error_info.log", std::ios::app);
				outfile_info << t << ": " << msg << std::endl;
				break ;
			}
			case ERROR_WARNING:
			{
				std::ofstream outfile_info("./log/error_info.log", std::ios::app);
				outfile_info << t << ": " << msg << std::endl;
				std::ofstream outfile_warning("./log/error_warning.log", std::ios::app);
				outfile_warning << t << ": " << msg << std::endl;
				break ;
			}
			default:
				break;
		}

	}
}