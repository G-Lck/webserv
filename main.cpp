#include "./includes/WebServ.hpp"

int	main( void )
{
	Server myServer;
	Socket *mySocket = new Socket;
	mySocket->setConfig();
	try
	{
		mySocket->makeSocket();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	myServer.addSocket(mySocket);
	myServer.epollInit();
	myServer.epollAddSockets();
	myServer.initServer();
	delete mySocket;
}
