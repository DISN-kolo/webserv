#include "../inc/Server.hpp"

Server::Server()
{
	_config = new ServerConfig();
	_perConnArr = std::vector<Connect * >(BLOG_SIZE, NULL);
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

void	Server::_onHeadLocated(int i, int *fdp)
{
	std::cout << std::setw(4) << i << " > " << std::flush;
	std::cout << "Head located. Stop reading for a moment" << std::endl;
	std::cout << std::setw(4) << i << " > " << std::flush;
	std::cout << "Total msg:" << std::endl;
	std::cout << _localRecvBuffers[i] << std::flush;
	// TODO responder class
	try
	{
		RequestHeadParser		req(_localRecvBuffers[i]);
		if (req.getMethod() == "POST")
		{
			// a body is a must-have then?
			// well, yeah iirc...
			// so this means we won't enter this function anymore I think.
			// we need to .erase from the newline x2 mark, and start taking in the body
			// but that's something to consider for the run function
			_perConnArr[i]->setNeedsBody(true);
			_perConnArr[i]->setContLen(req.getContLen());
			_perConnArr[i]->setKeepAlive(req.getKeepAlive()); // XXX stopped here :)
			// this vvvvvvvvvvvvvvvvvvvvvvvvvvv is for nlx2 erasure
			// TODO move this string array to be const like in the server.hpp or something idk idc
			size_t		nlnl;
			std::string	nls[4];
			nls[0] = CRLFCRLF;
			nls[1] = LFCRLF;
			nls[2] = CRLFLF;
			nls[3] = LFLF;
			for (int nli = 0; nli < 4; nli++)
			{
				nlnl = _localRecvBuffers[i].find(nls[nli]);
				if (nlnl != std::string::npos)
				{
					_localRecvBuffers[i].erase(0, nlnl + nls[nli].size());
					break ;
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ done erasing nlx2
		}
		else
		{
			// TODO filing notes
			// if it's not a post and we already have a head, we should just stop --drop and roll--
			// stop and send, I mean. or, you know, generate a proper response, cgi and all that jazz,
			// if needed. I'm a bit scared of all the fopen business tho. where the hell do I store the
			// fds for REAL FILES ON DISK for them to be included in poll? obviously, in the pollfd
			// array. Thus, it needs to be not of BLOG_SIZE in len, but like x2 (to account for file-reading).
			// and like add a new counter for "real open files" to not mess them up when doing a loop thru
			// the fds that got poll'd.
			// ALSO check out .... file writing! File writing is done via polling, too. No?
			ResponseGenerator	responseObject(200);

			send(*fdp, responseObject.getText().c_str(), responseObject.getSize(), 0);

			std::cout << std::setw(4) << i << " > " << std::flush;
			std::cout << "sent:" << std::endl;
			std::cout << responseObject.getText() << std::flush;
		}
	}
	catch (std::exception & e)
	{
		_perConnArr[i].setNeedsBody(false);
		std::cout << std::setw(4) << i << " > " << std::flush;
		std::cout << e.what() << std::endl;
		ResponseGenerator	responseObject(e.what());

		send(*fdp, responseObject.getText().c_str(), responseObject.getSize(), 0);

		std::cout << std::setw(4) << i << " > " << std::flush;
		std::cout << "sent:" << std::endl;
		std::cout << responseObject.getText() << std::flush;
	}
	if (!_perConnArr[i].getNeedsBody())
	{
		_localRecvBuffers[i].clear();
	}
	bool	keepalive = false; // TODO I beg you, parse the request.
	if (!keepalive)
	{
		close(*fdp);
		*fdp = -1;
		_compressTheArr = true;
	}
}

void	Server::run(void)
{
	std::vector<int>	lPorts = _config->getPorts();
	sockaddr_in	addresses[lPorts.size()];
	for (unsigned long i = 0; i < lPorts.size(); i++)
	{
		// open some abstract socket
		// and also make it nonblocking
		_listenSock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		if (_listenSock < 0)
			throw socketUnopenedError();

		// make it reusable
		_reuseAddressValue = 1;
		if (setsockopt(_listenSock, SOL_SOCKET, SO_REUSEADDR, &_reuseAddressValue, sizeof(_reuseAddressValue)) < 0)
			throw sockOptError();

		// make a struct that has the type of the address, the ip, the port
		std::memset(&(addresses[i]), 0, sizeof (addresses[i]));
		addresses[i].sin_family = AF_INET;
		addresses[i].sin_addr.s_addr = htonl(INADDR_ANY);
		// TODO maybe thru the config not "any" ----^^^
		// TODO need to make a function to convert a.b.c.d into whatever s_addr is
		addresses[i].sin_port = htons(lPorts[i]);
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

	// _timeout is measured in ms, but here we go infinitely
	_timeout = -1;
	_socksN = _listenSocks.size();
	_lstnN = _socksN;
	// newconnect's -2 is its init value while -1 indicates a fail
	_newConnect = -2;
	_compressTheArr = false;
	char	buf[RBUF_SIZE + 1];
	// this will store what we read because we shall be able to read in multiple passes before closing the connection.
	_localRecvBuffers = std::vector<std::string>(BLOG_SIZE, "");

	std::cout << "Alright, starting. Ports: " << *(lPorts.begin()) << ".." << *(lPorts.end() - 1) << std::endl;
	_running = true;
	bool	keepalive; // XXX TODO
	keepalive = false;
	while (_running)
	{
		_retCode = poll(socks, _socksN, _timeout);
		if (_retCode < 0)
			throw pollError();
//		else if (_retCode == 0)
//		{
//			std::cout << "Poll timeout. Byeeeee" << std::endl;
//			return ;
//		}
		std::cout << "Just poll'd, socks number is " << _socksN << std::endl;

		// go thru all the socks that could have possibly been affected
		// see if they've been affected
		_curSize = _socksN;
		for (int i = 0; i < _curSize; i++)
		{
			if (socks[i].revents == 0)
			{
				std::cout << "revents on " << i << " is 0" << std::endl;
				continue ;
			}
			else if (((socks[i].revents & POLLERR) == POLLERR) || ((socks[i].revents & POLLHUP) == POLLHUP) || ((socks[i].revents & POLLNVAL) == POLLNVAL))
			{
				if (i < _lstnN)
				{
					std::cout << std::setw(4) << i << " > " << std::flush;
					std::cout << "_listenSock got a pollerr or a pollhup or an nval" << std::endl;
					continue ;
				}
				std::cout << std::setw(4) << i << " > " << std::flush;
				std::cout << "got an err/hup/val" << std::endl;
				close(socks[i].fd);
				socks[i].fd = -1;
				delete _perConnArr[i];
				_perConnArr[i] = NULL;
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
								_perConnArr[j] = new Connect;
								break ;
							}
						}
						_newConnect = accept(_listenSocks[i], NULL, NULL);
					}
				}
				else if (socks[i].fd != -1)
				{
//					std::cout << "Descriptior " << socks[i].fd << " at pos " << i << " readable" << std::endl;
					for (int j = 0; j < RBUF_SIZE + 1; j++)
						buf[j] = 0;
					_retCode = recv(socks[i].fd, buf, RBUF_SIZE, 0);
					if (_retCode < 0)
					{
						// to crash or not to crash? XXX to think
//						throw readError();
						close(socks[i].fd);
						socks[i].fd = -1;
						_compressTheArr = true;
						delete  _perConnArr[i];
						_perConnArr[i] = NULL;
						std::cout << std::setw(4) << i << " > " << std::flush;
						std::cout << "_retCode " << _retCode << " but since we're stupid we're gonna do nothing special. OR ARE WE??? (stupid i mean)" << std::endl;
					}
					else if (_retCode == 0)
					{
						std::cout << std::setw(4) << i << " > " << std::flush;
						std::cout << "_retCode is 0, but poll says POLLIN and no errors(?). Maybe we forgot to clean up?" << std::endl;
						// how do we even get here.......
						if (_localRecvBuffers[i].size() != 0)
						{
							// well, whatever. yk what's also important here? if we have an ogoing connection that needs a body, well, there should be a secret third option
							// that gets checked first despite being the, uh third option. 
							if (_perConnArr[i].getNeedsBody())
							{
								// TODO make ts a function
								std::cout << std::setw(4) << i << " > " << std::flush;
								std::cout << "Is that some sort of a joke?" << std::endl;
								std::cout << std::setw(4) << i << " > " << std::flush;
								std::cout << "No double-enndline, AND the buff isn't zero... But nothing to read." << std::endl;
								std::cout << std::setw(4) << i << " > " << std::flush;
								std::cout << "Here should be a timeout thing instead, probably." << std::endl;
								// TODO -----------------------^^^^^^^ (?)
								std::cout << std::setw(4) << i << " > " << std::flush;
								std::cout << "What's worse the client wants to send a body in. Where?" << std::endl;
								// TODO make ts a function
								close(socks[i].fd);
								socks[i].fd = -1;
								_compressTheArr = true;
								delete  _perConnArr[i];
								_perConnArr[i] = NULL;
							}
							else if (_localRecvBuffers[i].find(CRLFCRLF) != std::string::npos ||
									_localRecvBuffers[i].find(LFCRLF) != std::string::npos ||
									_localRecvBuffers[i].find(CRLFLF) != std::string::npos ||
									_localRecvBuffers[i].find(LFLF) != std::string::npos)
							{
								_onHeadLocated(i, &(socks[i].fd));
							}
							else
							{
								std::cout << std::setw(4) << i << " > " << std::flush;
								std::cout << "Is that some sort of a joke?" << std::endl;
								std::cout << std::setw(4) << i << " > " << std::flush;
								std::cout << "No double-enndline, AND the buff isn't zero... But nothing to read." << std::endl;
								std::cout << std::setw(4) << i << " > " << std::flush;
								std::cout << "Here should be a timeout thing instead, probably." << std::endl;
								// TODO -----------------------^^^^^^^ (?)
								close(socks[i].fd);
								socks[i].fd = -1;
								_compressTheArr = true;
								delete  _perConnArr[i];
								_perConnArr[i] = NULL;
							}
						}
						else
						{
							std::cout << std::setw(4) << i << " > " << std::flush;
							std::cout << "Is that some sort of a joke?" << std::endl;
							std::cout << std::setw(4) << i << " > " << std::flush;
							std::cout << "Nothing in local buf, the read is 0, how tf did we even get polled????" << std::endl;
							close(socks[i].fd);
							socks[i].fd = -1;
							_compressTheArr = true;
							delete  _perConnArr[i];
							_perConnArr[i] = NULL;
						}
					}
					else if (_retCode > 0)
					{
						// TODO: bare CR to SP replace
						// (?)
						// https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-4
						std::cout << std::setw(4) << i << " > " << std::flush;
						std::cout << "Message on " << i << " received (maybe) partially, " << _retCode << " bytes, RBUF (w/o \\0) is " << RBUF_SIZE << "." << std::endl;
						std::cout << std::setw(4) << i << " > " << std::flush;
						std::cout << "received:" << std::endl;
						buf[_retCode] = 0;
						std::cout << buf << std::endl;
						_localRecvBuffers[i] += std::string(buf);

						std::cout << std::setw(4) << i << " > " << std::flush;
						std::cout << "Checking for a double line-break (any combo of LF and CRLF)" << std::endl;
						if (_perConnArr[i].getNeedsBody())
						{
							if (_localRecvBuffers[i].size > _perConnArr[i].getContLen())
							{
								// TODO error content too long or sumn
							}
							else if (_localRecvBuffers[i].size == _perConnArr[i].getContLen())
							{
								// TODO epic victory, send response ig
								// write the file
								// don't forge IO multiplexing xdddddddddddddddddd kill me
							}
						}
						else if (_localRecvBuffers[i].find(CRLFCRLF) != std::string::npos ||
								_localRecvBuffers[i].find(LFCRLF) != std::string::npos ||
								_localRecvBuffers[i].find(CRLFLF) != std::string::npos ||
								_localRecvBuffers[i].find(LFLF) != std::string::npos)
						{
							_onHeadLocated(i, &(socks[i].fd));
						}
						else
						{
							std::cout << std::setw(4) << i << " > " << std::flush;
							std::cout << "Continue reading thru the next cycle!!!" << std::endl;
						}
					}
					// keep-alive check, pls TODO
//					close(socks[i].fd);
//					socks[i].fd = -1;
//					_compressTheArr = true;
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
						_localRecvBuffers[j] = _localRecvBuffers[j + 1];
						_perConnArr[j] = _perConnArr[j + 1];
						socks[j].fd = socks[j + 1].fd;
						socks[j].events = socks[j + 1].events;
						socks[j].revents = socks[j + 1].revents;
					}
					i--;
				}
			}
			for (int k = _socksN; k < BLOG_SIZE; k++)
			{
				_localRecvBuffers[k].clear();
				delete _perConnArr[k];
				_perConnArr[k] = NULL;
				socks[k].fd = -1;
				socks[k].events = 0;
				socks[k].revents = 0;
			}
		}
		std::cout << "                one cycle done!" << std::endl;
	} /* while (_running) */
}

Server::~Server()
{
	delete _config;
}
