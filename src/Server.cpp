#include "../inc/Server.hpp"

Server::Server()
{
	_config = new ServerConfig();
}

Server::Server(const Server & obj)
{
	(void)obj;
}

Server &Server::operator=(const Server & obj)
{
	(void)obj;
	return (*this);
}

void	Server::run(void) const
{
	int	listenSock;
	std::vector<int> listenSocks;
	int	reuseAddressValue;
	sockaddr_in	addresses[_config->getPorts().size()];
	for (unsigned long i = 0; i < _config->getPorts().size(); i++)
	{
		// open some abstract socket
		// and also make it nonblocking
		listenSock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		if (listenSock < 0)
			throw socketUnopenedError();

		// make it reusable
		reuseAddressValue = 1;
		if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &reuseAddressValue, sizeof(reuseAddressValue)) < 0)
			throw sockOptError();

		// make a struct that has the type of the address, the ip, the port
		std::memset(&(addresses[i]), 0, sizeof (addresses[i]));
		addresses[i].sin_family = AF_INET;
		addresses[i].sin_addr.s_addr = htonl(INADDR_ANY);
		// TODO maybe thru the config not "any" ----^^^
		addresses[i].sin_port = htons(_config->getPorts()[i]);
		// make a sock be of a specified address
		if (bind(listenSock, (struct sockaddr *)&(addresses[i]), sizeof(addresses[i])) < 0)
			throw bindError();

		// listen up!
		if (listen(listenSock, BLOG_SIZE) < 0)
			throw listenError();

		listenSocks.push_back(listenSock);
	}
	// make an array of poll's structs
	struct pollfd	socks[BLOG_SIZE];
//	std::memset(socks, 0, sizeof (socks));
	for (int x = 0; x < BLOG_SIZE; x++)
	{
		socks[x].fd = -1;
		socks[x].events = 0;
		socks[x].revents = 0;
	}

	// setup init listening sock in the array of poll's structs
	for (unsigned long i = 0; i < listenSocks.size(); i++)
	{
		socks[i].fd = listenSocks[i];
		socks[i].events = POLLIN;
	}
	int			lstnN = listenSocks.size();

	// timeout is measured in ms
	int 		timeout = 3 * 60 * 1000;
	int			socksN = listenSocks.size();
	int			curSize;
	bool		running = true;
	// -2 is its init value while -1 indicates a fail
	int			newConnect = -2;
	char		buf[RBUF_SIZE];
	std::string	response;
	bool		compressTheArr = false;
	int			retCode;
	std::cout << "Alright, starting. Ports: " << port1 << ".." << port2 << std::endl;
	while (running)
	{
		retCode = poll(socks, socksN, timeout);
//		retCode = poll(socks, socksN, 0);
		if (retCode < 0)
			throw pollError();
		else if (retCode == 0)
		{
			// this shouldn't happen in the real webserv ? XXX
			std::cout << "Poll timeout. Byeeeee" << std::endl;
			return (1);
		}
		std::cout << "Just poll'd, socks number is " << socksN << std::endl;
//		std::cout << "meanwhile, the current first socksN socks are like this:" << std::endl;
		for (int i = 0; i < socksN; i++)
		{
//			std::cout << std::setw(5) << socks[i].fd << " | ";
//			std::cout << std::setw(15) << pe[socks[i].events] << " | ";
//			std::cout << std::setw(15) << pe[socks[i].revents] << std::endl;
		}

		// go thru all the socks that could have possibly been affected
		// see if they've been affected
		curSize = socksN;
		for (int i = 0; i < curSize; i++)
		{
			if (socks[i].revents == 0)
			{
//				std::cout << "revents on " << i << " is 0" << std::endl;
				continue ;
			}
			else if (((socks[i].revents & POLLERR) == POLLERR) || ((socks[i].revents & POLLHUP) == POLLHUP) || ((socks[i].revents & POLLNVAL) == POLLNVAL))
			{
				if (i < lstnN)
				{
//					std::cout << "listenSock got a pollerr or a pollhup" << std::endl;
					continue ;
				}
				close(socks[i].fd);
				socks[i].fd = -1;
				//socks[i].events = 0;
				compressTheArr = true;
			}
			else if ((socks[i].revents & POLLIN) == POLLIN)
			{
				/// XXX
//				if (socks[i].fd == listenSock)
				if (i < lstnN)
				{
					// wow is that the listening fd? our sock? yes it is!
//					std::cout << "listenSock readable" << std::endl;
					newConnect = accept(listenSocks[i], NULL, NULL);
					//XXX okay, what if we don't loop over the listen sock? seems to duplicate fds :/
					while (newConnect > 0)
					//if (newConnect > 0)
					{
//						std::cout << "Accepted to " << newConnect << std::endl;
						for (int j = lstnN; j < BLOG_SIZE; j++)
						{
							if (socks[j].fd == -1)
							{
								socksN++;
								socks[j].fd = newConnect;
								socks[j].events = POLLIN;
								break ;
							}
						}
						newConnect = accept(listenSocks[i], NULL, NULL);
					}
//					std::cout << "After accepting all these wonderful new connections, our list looks like this:" << std::endl;
					for (int i = 0; i < socksN; i++)
					{
//						std::cout << std::setw(5) << socks[i].fd << " | ";
//						std::cout << std::setw(15) << pe[socks[i].events] << " | ";
//						std::cout << std::setw(15) << pe[socks[i].revents] << std::endl;
					}
				}
				else if (socks[i].fd != -1)
				{
//					std::cout << "Descriptior " << socks[i].fd << " at pos " << i << " readable" << std::endl;
					for (int j = 0; j < RBUF_SIZE; j++)
						buf[j] = 0;
					retCode = recv(socks[i].fd, buf, sizeof(buf), 0);
					if (retCode < 0)
					{
//						std::cout << "retCode " << retCode << "but since we're stupid we're gonna do nothing special. OR ARE WE??? (stupid i mean)" << std::endl;
//						throw readError();
					}
					else if (retCode == 0)
					{
//						std::cout << "Conn " << socks[i].fd << " at " << i << " has 0 to read" << std::endl;
					}
					else
					{
						std::cout << "received:" << std::endl;
						std::cout << buf << std::endl;
						response = responseGenerator(200);

						send(socks[i].fd, response.c_str(), response.size(), 0);
						// ????
//						send(socks[i].fd, response.c_str(), response.size() + 1, 0);
						// ????

						std::cout << "sent:" << std::endl;
						std::cout << response << std::endl;
					}
					close(socks[i].fd);
					socks[i].fd = -1;
					//socks[i].events = 0;
					compressTheArr = true;
				}
			}
		} /* for to iterate thru socks upon poll's return */
		// lol tbh this seems to waste some time :) idk
		if (compressTheArr)
		{
//			std::cout << "before compression" << std::endl;
			for (int i = 0; i < socksN; i++)
			{
//				std::cout << std::setw(5) << socks[i].fd << " | ";
//				std::cout << std::setw(15) << pe[socks[i].events] << " | ";
//				std::cout << std::setw(15) << pe[socks[i].revents] << std::endl;
			}
			compressTheArr = false;
			for (int i = lstnN; i < socksN; i++)
			{
				// this became useful :)
				if (socks[i].fd == -1)
				{
					socksN--;
					for (int j = i; j < socksN; j++)
					{
						socks[j].fd = socks[j + 1].fd;
						socks[j].events = socks[j + 1].events;
						socks[j].revents = socks[j + 1].revents;
					}
					i--;
				}
			}
//			std::cout << "after compression" << std::endl;
			for (int i = 0; i < socksN; i++)
			{
//				std::cout << std::setw(5) << socks[i].fd << " | ";
//				std::cout << std::setw(15) << pe[socks[i].events] << " | ";
//				std::cout << std::setw(15) << pe[socks[i].revents] << std::endl;
			}
//			std::cout << "cleanup the rest of the socks..." << std::endl;
			for (int k = socksN; k < BLOG_SIZE; k++)
			{
				socks[k].fd = -1;
				socks[k].events = 0;
				socks[k].revents = 0;
			}
		}
//		std::cout << "one cycle done!" << std::endl;
	} /* while (running) */
}

Server::~Server()
{
	delete _config;
}
