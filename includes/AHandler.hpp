#ifndef AHANDLER_HPP
#define AHANDLER_HPP

class AHandler
{
	private:
		Client	_client;
	public:
		AHandler();
		AHandler(const AHandler& other);
		AHandler& operator=(const AHandler& other);
		~AHandler();
};

#endif