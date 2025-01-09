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
	sockaddr_in	addresses[_config->getPorts().size()];
	for (unsigned long i = 0; i < _config->getPorts().size(); i++)
	{
		// open some abstract socket
		// and also make it nonblocking
		_listenSock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		if (_listenSock < 0)
			throw socketUnopenedError();

		// make it reusable
		reuseAddressValue = 1;
		if (setsockopt(_listenSock, SOL_SOCKET, SO_REUSEADDR, &reuseAddressValue, sizeof(reuseAddressValue)) < 0)
			throw sockOptError();

		// make a struct that has the type of the address, the ip, the port
		std::memset(&(addresses[i]), 0, sizeof (addresses[i]));
		addresses[i].sin_family = AF_INET;
		addresses[i].sin_addr.s_addr = htonl(INADDR_ANY);
		// TODO maybe thru the config not "any" ----^^^
		addresses[i].sin_port = htons(_config->getPorts()[i]);
		// make a sock be of a specified address
		if (bind(_listenSock, (struct sockaddr *)&(addresses[i]), sizeof(addresses[i])) < 0)
			throw bindError();

		// listen up!
		if (listen(_listenSock, BLOG_SIZE) < 0)
			throw listenError();

		_listenSocks.push_back(_listenSock);
	}
	// make an array of poll's structs
	struct pollfd	socks[BLOG_SIZE];
	for (int x = 0; x < BLOG_SIZE; x++)
	{
		socks[x].fd = -1;
		socks[x].events = 0;
		socks[x].revents = 0;
	}

	// setup init listening sock in the array of poll's structs
	for (unsigned long i = 0; i < _listenSocks.size(); i++)
	{
		socks[i].fd = _listenSocks[i];
		socks[i].events = POLLIN;
	}

	// _timeout is measured in ms
	_timeout = 3*60*1000;
	_socksN = _listenSocks.size();
	_lstnN = _socksN;
	_newConnect = -2;
	// newconnect's -2 is its init value while -1 indicates a fail
	_compressTheArr = false;
	char	buf[RBUF_SIZE];
	std::cout << "Alright, starting. Ports: " << *(_config->getPorts().begin()) << ".." << *(_config->getPorts().end() - 1) << std::endl;
	running = true;
	while (running)
	{
		_retCode = poll(socks, _socksN, _timeout);
//		_retCode = poll(socks, _socksN, 0);
		// idk. we shall live forever? nonblockingly? TODO
		if (_retCode < 0)
			throw pollError();
		else if (_retCode == 0)
		{
			std::cout << "Poll timeout. Byeeeee" << std::endl;
			return ;
		}
		std::cout << "Just poll'd, socks number is " << _socksN << std::endl;

		// go thru all the socks that could have possibly been affected
		// see if they've been affected
		curSize = _socksN;
		for (int i = 0; i < curSize; i++)
		{
			if (socks[i].revents == 0)
			{
//				std::cout << "revents on " << i << " is 0" << std::endl;
				continue ;
			}
			else if (((socks[i].revents & POLLERR) == POLLERR) || ((socks[i].revents & POLLHUP) == POLLHUP) || ((socks[i].revents & POLLNVAL) == POLLNVAL))
			{
				if (i < _lstnN)
				{
//					std::cout << "_listenSock got a pollerr or a pollhup" << std::endl;
					continue ;
				}
				close(socks[i].fd);
				socks[i].fd = -1;
				//socks[i].events = 0;
				_compressTheArr = true;
			}
			else if ((socks[i].revents & POLLIN) == POLLIN)
			{
				if (i < _lstnN)
				{
					// wow is that the listening fd? our sock? yes it is!
					_newConnect = accept(_listenSocks[i], NULL, NULL);
					while (_newConnect > 0)
					{
//						std::cout << "Accepted to " << _newConnect << std::endl;
						for (int j = _lstnN; j < BLOG_SIZE; j++)
						{
							if (socks[j].fd == -1)
							{
								_socksN++;
								socks[j].fd = _newConnect;
								socks[j].events = POLLIN;
								break ;
							}
						}
						_newConnect = accept(_listenSocks[i], NULL, NULL);
					}
				}
				else if (socks[i].fd != -1)
				{
//					std::cout << "Descriptior " << socks[i].fd << " at pos " << i << " readable" << std::endl;
					for (int j = 0; j < RBUF_SIZE; j++)
						buf[j] = 0;
					_retCode = recv(socks[i].fd, buf, sizeof(buf), 0);
					if (_retCode < 0)
					{
//						std::cout << "_retCode " << _retCode << "but since we're stupid we're gonna do nothing special. OR ARE WE??? (stupid i mean)" << std::endl;
//						throw readError();
					}
					else if (_retCode == 0)
					{
//						std::cout << "Conn " << socks[i].fd << " at " << i << " has 0 to read" << std::endl;
					}
					else
					{
						std::cout << "received:" << std::endl;
						std::cout << buf << std::endl;
						// TODO responder class
						_response = _responseGenerator(200);

						send(socks[i].fd, _response.c_str(), _response.size(), 0);
						//                                               XXX?????XXX
//						send(socks[i].fd, _response.c_str(), _response.size() + 1, 0);
						//                                               XXX?????XXX

						std::cout << "sent:" << std::endl;
						std::cout << _response << std::endl;
					}
					close(socks[i].fd);
					socks[i].fd = -1;
					_compressTheArr = true;
				}
			}
		} /* for to iterate thru socks upon poll's return */
		if (_compressTheArr)
		{
			_compressTheArr = false;
			for (int i = _lstnN; i < _socksN; i++)
			{
				if (socks[i].fd == -1)
				{
					_socksN--;
					for (int j = i; j < _socksN; j++)
					{
						socks[j].fd = socks[j + 1].fd;
						socks[j].events = socks[j + 1].events;
						socks[j].revents = socks[j + 1].revents;
					}
					i--;
				}
			}
			for (int k = _socksN; k < BLOG_SIZE; k++)
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
