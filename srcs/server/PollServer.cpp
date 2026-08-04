#include "../../includes/PollServer.hpp"

static bool hasActiveCgiForClientFd(const std::map<int, CgiHandler*> &cgi_handlers, int client_fd)
{
	std::map<int, CgiHandler*>::const_iterator it = cgi_handlers.begin();
	while (it != cgi_handlers.end())
	{
		CgiHandler *cgi = it->second;
		if (cgi != NULL && cgi->getClient() != NULL && cgi->getClient()->getFd() == client_fd)
			return true;
		++it;
	}
	return false;
}

// --------------- Orthodoxy ---------------

PollServer::PollServer() {}

PollServer::~PollServer()
{
	clearPollState();
}

// --------------- Exceptions ---------------

PollServer::runtimePollServerException::runtimePollServerException(const char* message)
	: std::runtime_error(message) {}

// --------------- Set up Poll ---------------

void	PollServer::initMultiplexer(void)
{
	clearPollState();
}

void	PollServer::addSocketsToMultiplexer(void)
{
	std::map<int, Socket*>::iterator it = this->_sockets.begin();
	while (it != this->_sockets.end())
	{
		struct pollfd pfd;
		pfd.fd = it->second->getFd();
		pfd.events = POLLIN;
		pfd.revents = 0;
		this->_fd_to_poll_index[pfd.fd] = this->_pollfds.size();
		this->_pollfds.push_back(pfd);
		++it;
	}
}

void	PollServer::run(void)
{
	if (this->_pollfds.empty())
		throw runtimePollServerException("Error\nNo file descriptor to poll.");
	while (1)
	{
		struct pollfd *pollfds = this->_pollfds.empty() ? NULL : &this->_pollfds[0];
		int ready_count = poll(pollfds, this->_pollfds.size(), -1);
		if (ready_count == -1)
		{
			if (errno == EINTR)
				continue;
			throw runtimePollServerException((std::string("Error fatal:\n") + strerror(errno)).c_str());
		}

		std::vector< std::pair<int, short> > triggered;
		for (size_t i = 0; i < this->_pollfds.size(); ++i)
		{
			if (this->_pollfds[i].revents == 0)
				continue;
			triggered.push_back(std::make_pair(this->_pollfds[i].fd, this->_pollfds[i].revents));
			this->_pollfds[i].revents = 0;
		}

		for (size_t i = 0; i < triggered.size(); ++i)
		{
			int current_fd = triggered[i].first;
			short events = triggered[i].second;

			if (this->fdIsCgi(current_fd))
			{
				this->handleCgiEvent(current_fd, events);
				continue;
			}

			if (this->fdMatch(current_fd))
			{
				if (events & POLLIN)
					this->addNewClient(current_fd);
				continue;
			}

			if (this->_clients.find(current_fd) == this->_clients.end())
				continue;

			if (hasActiveCgiForClientFd(this->_cgi_handlers, current_fd))
				continue;

			if (events & (POLLERR | POLLNVAL))
			{
				this->clientDisconnect(current_fd);
				continue;
			}
			if (events & POLLIN)
				this->refreshClientTime(current_fd);
			if (events & POLLIN)
				this->readRequest(current_fd);
			if (this->_clients.find(current_fd) == this->_clients.end())
				continue;
			if (events & POLLOUT)
				this->refreshClientTime(current_fd);
			if (events & POLLOUT)
				this->sendResponse(current_fd);
			if (this->_clients.find(current_fd) == this->_clients.end())
				continue;

			if (events & POLLHUP)
			{
				Client *c = this->_clients[current_fd];
				if (hasActiveCgiForClientFd(this->_cgi_handlers, current_fd))
					continue;
				if (c->isWriteBufferEmpty() && c->isResponseQueueEmpty())
					this->clientDisconnect(current_fd);
			}
		}
		std::map<int, CgiHandler*>::iterator cgi_it = this->_cgi_handlers.begin();
		while (cgi_it != this->_cgi_handlers.end())
		{
			if (cgi_it->second != NULL && cgi_it->second->getClient() != NULL)
				this->refreshClientTime(cgi_it->second->getClient()->getFd());
			++cgi_it;
		}
		monitorClients();
		monitorCGI();
	}
}

void	PollServer::handleCgiEvent(int fd, short poll_events)
{
	CgiHandler* cgi = this->getCgiHandler(fd);

	if (cgi == NULL)
		return;
	int stdin_fd = cgi->getParentFdOut();
	int stdout_fd = cgi->getParentFdIn();
	if ((poll_events & POLLOUT) && fd == cgi->getParentFdOut())
	{
		cgi->continueWriting();
	}
	else if ((poll_events & POLLIN) && fd == cgi->getParentFdIn())
	{
		cgi->continueReading();
	}
	if (cgi->isFinished())
	{
		if (stdin_fd != -1)
			this->removeFdFromMultiplexer(stdin_fd);
		if (stdout_fd != -1 && stdout_fd != stdin_fd)
			this->removeFdFromMultiplexer(stdout_fd);
		this->cgiDisconnect(stdout_fd);
	}
}

bool	PollServer::addFdToMultiplexer( int fd )
{
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	this->_fd_to_poll_index[fd] = this->_pollfds.size();
	this->_pollfds.push_back(pfd);
	return true;
}

void	PollServer::removeFdFromMultiplexer( int fd )
{
	std::map<int, size_t>::iterator it = this->_fd_to_poll_index.find(fd);
	if (it == this->_fd_to_poll_index.end())
		return;

	size_t index = it->second;
	size_t last_index = this->_pollfds.size() - 1;
	if (index != last_index)
	{
		this->_pollfds[index] = this->_pollfds[last_index];
		this->_fd_to_poll_index[this->_pollfds[index].fd] = index;
	}
	this->_pollfds.pop_back();
	this->_fd_to_poll_index.erase(it);
}

void	PollServer::watchForRead( int fd )
{
	std::map<int, size_t>::iterator it = this->_fd_to_poll_index.find(fd);
	if (it == this->_fd_to_poll_index.end())
		return;
	this->_pollfds[it->second].events = POLLIN;
}

void	PollServer::watchForWrite( int fd )
{
	std::map<int, size_t>::iterator it = this->_fd_to_poll_index.find(fd);
	if (it == this->_fd_to_poll_index.end())
		return;
	this->_pollfds[it->second].events = POLLOUT;
}

void	PollServer::clearPollState(void)
{
	this->_pollfds.clear();
	this->_fd_to_poll_index.clear();
}
