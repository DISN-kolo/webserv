#include "../inc/Server.hpp"

Server::Server()
{
	_rbufSize = 4096;
	_sbufSize = 4096;
	_blogSize = 4096;
	_connsAmt = 4096;
	_config = new ServerConfig();
	_perConnArr = std::vector<Connect * >(_connsAmt, NULL);
	_nls.push_back(CRLFCRLF);
	_nls.push_back(LFCRLF);
	_nls.push_back(CRLFLF);
	_nls.push_back(LFLF);
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

void	Server::_debugMsgI(int i, std::string msg)
{
	std::cout << std::setw(4) << i << " > " << std::flush;
	std::cout << msg << std::endl;
}

void	Server::_debugMsgTimeI(int i, time_t curTime)
{
	std::cout << std::setw(4) << i << " > " << std::flush;
	std::cout << "time started: " << _perConnArr[i]->getTimeStarted() << ", diff with now: " << curTime - _perConnArr[i]->getTimeStarted() << std::endl;
}

void	Server::_eraseDoubleNlInLocalRecvBuffer(int i)
{
	for (int nli = 0; nli < 4; nli++)
	{
		_nlnl = _localRecvBuffers[i].find(_nls[nli]);
		if (_nlnl != std::string::npos)
		{
			_localRecvBuffers[i].erase(0, _nlnl + _nls[nli].size());
			break ;
		}
	}
}

void	Server::_purgeOneConnection(int i, int *fdp)
{
	close(*fdp);
	*fdp = -1;
	delete _perConnArr[i];
	_perConnArr[i] = NULL;
	_compressTheArr = true;
}

void	Server::_onHeadLocated(int i, int *fdp)
{
	_debugMsgI(i, "Head located. Stop reading for a moment");
	_debugMsgI(i, "Total msg:");
	std::cout << _localRecvBuffers[i] << std::flush;
	// in case RHP fails; keep-alive is the default for http 1.1
	// TODO responder class
	try
	{
		RequestHeadParser		req(_localRecvBuffers[i]);
		_perConnArr[i]->setKeepAlive(req.getKeepAlive());
		_perConnArr[i]->setKaTimeout(req.getKaTimeout());
		if (req.getMethod() == "POST")
		{
			// a body is a must-have then?
			// well, yeah iirc...
			// so this means we won't enter this function anymore I think.
			// we need to .erase up to the newline x2 mark (inclusive), and start taking in the body
			// but that's something to consider for the run function
			_perConnArr[i]->setNeedsBody(true);
			_perConnArr[i]->setContLen(req.getContLen());
			_eraseDoubleNlInLocalRecvBuffer(i);
		}
		else
		{
			// TODO filing notes
			// if it's not a post and we already have a head, we should just stop --drop and roll--
			// stop and send, I mean. or, you know, generate a proper response, cgi and all that jazz,
			// if needed. I'm a bit scared of all the fopen business tho. where the hell do I store the
			// fds for REAL FILES ON DISK for them to be included in poll? obviously, in the pollfd
			// array. Thus, it needs to be not of _blogSize in len, but like x2 (to account for file-reading).
			// and like add a new counter for "real open files" to not mess them up when doing a loop thru
			// the fds that got poll'd.
			// ALSO check out .... file writing! File writing is done via polling, too. No?
			// ALSO check out .... just sending regular responses might require poll?????? like.... POLLOUT n stuff....... oh my gaaaaaaawddddddddddd
			_perConnArr[i]->setNeedsBody(false);
			ResponseGenerator	responseObject(200);

			send(*fdp, responseObject.getText().c_str(), responseObject.getSize(), 0);
			_perConnArr[i]->setTimeStarted(time(NULL));
			std::cout << "reset starting time on " << i << std::endl;

			_debugMsgI(i, "sent:");
			std::cout << responseObject.getText() << std::flush;
			_eraseDoubleNlInLocalRecvBuffer(i);
		}
	}
	catch (std::exception & e)
	{
		_perConnArr[i]->setNeedsBody(false);
		_debugMsgI(i, e.what());
		ResponseGenerator	responseObject(e.what());

		send(*fdp, responseObject.getText().c_str(), responseObject.getSize(), 0);
		_perConnArr[i]->setTimeStarted(time(NULL));
		std::cout << "reset starting time on " << i << std::endl;

		_debugMsgI(i, "sent:");
		std::cout << responseObject.getText() << std::flush;
		_localRecvBuffers[i].clear();
	}
	if (!_perConnArr[i]->getKeepAlive())
	{
		if (!_perConnArr[i]->getNeedsBody())
		{
			_localRecvBuffers[i].clear();
			_purgeOneConnection(i, fdp);
		}
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
		if (listen(_listenSock, _blogSize) < 0)
			throw listenError();

		_listenSocks.push_back(_listenSock);
	}
	// make an array of poll's structs
	struct pollfd	socks[_connsAmt * 2];
	// looks like (max sizes. keep in mind that we'll be shrinking ts dynamically to help poll iterate thru only
	//the active fds instead of a mix of fds and -1s)
	// [0         ...      _lS.size) -- for listening sockets
	// [_lS.size  ...     _connsAmt) -- for accepted connections, where we recv() from.
	//we also send() there. so, after receiving a message up to the "time to send" part,
	//we must change the pollfd flag to POLLOUT
	// [_connsAmt ... _connsAmt * 2) -- for accepted connections, where we write the submitted files into.
	//probably, CGI related writes too, since they sorta replace the regular file-writing. and the read stuff,
	//too. because 1 conn = 1 write or 1 read maximum... so who cares, like.....
	for (int x = 0; x < _connsAmt * 2; x++)
	{
		socks[x].fd = -1;
		socks[x].events = 0;
		socks[x].revents = 0;
	}

	// setup init listening sock in the array of poll's structs
	for (size_t i = 0; i < _listenSocks.size(); i++)
	{
		socks[i].fd = _listenSocks[i];
		socks[i].events = POLLIN;
	}

	// _timeout is measured in ms
	_timeout = 1 * 1000;
	_socksN = _listenSocks.size();
	_lstnN = _socksN;
	// newconnect's -2 is its init value while -1 indicates a fail
	_newConnect = -2;
	_compressTheArr = false;
	char	buf[_rbufSize + 1];
	// this will store what we read because we shall be able to read in multiple passes before closing the connection.
	_localRecvBuffers = std::vector<std::string>(_connsAmt, "");

	std::cout << "Alright, starting. Ports: " << *(lPorts.begin()) << ".." << *(lPorts.end() - 1) << std::endl;
	_running = true;
	time_t	curTime;
	while (_running)
	{
		_retCode = poll(socks, _socksN, _timeout);
		curTime = time(NULL);
		if (_retCode < 0)
		{
			throw pollError();
		}
		else if (_retCode == 0)
		{
			for (int i = _lstnN; i < _socksN; i++)
			{
				if (_perConnArr[i] != NULL)
				{
					if (_perConnArr[i]->getKaTimeout() < curTime - _perConnArr[i]->getTimeStarted())
					{
						_localRecvBuffers[i].clear();
						_purgeOneConnection(i, &(socks[i].fd));
					}
				}
			}
		}
		std::cout << "Just poll'd, socks number is " << _socksN << std::endl;

		// go thru all the socks that could have possibly been affected
		// see if they've been affected
		_curSize = _socksN;
		for (int i = 0; i < _curSize; i++)
		{
			if (socks[i].fd == -1)
				continue ;
			if (_perConnArr[i] != NULL)
			{
				_debugMsgTimeI(i, curTime);
				if (!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < curTime - _perConnArr[i]->getTimeStarted())
				{
					_localRecvBuffers[i].clear();
					_purgeOneConnection(i, &(socks[i].fd));
				}
			}
			if (socks[i].revents == 0)
			{
				std::cout << "revents on " << i << " is 0" << std::endl;
				continue ;
			}
			else if (((socks[i].revents & POLLERR) == POLLERR) || ((socks[i].revents & POLLHUP) == POLLHUP) || ((socks[i].revents & POLLNVAL) == POLLNVAL))
			{
				if (i < _lstnN)
				{
					_debugMsgI(i, "_listenSock got a pollerr or a pollhup or an nval");
					continue ;
				}
				_debugMsgI(i, "got an err/hup/val");
				_purgeOneConnection(i, &(socks[i].fd));
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
						for (int j = _lstnN; j < _blogSize; j++)
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
				else
				{
//					std::cout << "Descriptior " << socks[i].fd << " at pos " << i << " readable" << std::endl;
					for (int j = 0; j < _rbufSize + 1; j++)
						buf[j] = 0;
					_retCode = recv(socks[i].fd, buf, _rbufSize, 0);
					if (_retCode < 0)
					{
						// to crash or not to crash? XXX to think
//						throw readError();
						_purgeOneConnection(i, &(socks[i].fd));
						std::cout << std::setw(4) << i << " > " << std::flush;
						std::cout << "_retCode " << _retCode << " but since we're stupid we're gonna do nothing special. OR ARE WE??? (stupid i mean)" << std::endl;
					}
					else if (_retCode == 0)
					{
						_debugMsgI(i, "_retCode is 0, but poll says POLLIN and no errors(?). Maybe we forgot to clean up?");
						// how do we even get here.......
						if (_localRecvBuffers[i].size() != 0)
						{
							// well, whatever. yk what's also important here? if we have an ogoing connection that needs a body, well, there should be a secret third option
							// that gets checked first despite being the, uh, "third option". oh, there it is!
							if (_perConnArr[i]->getNeedsBody())
							{
								std::cout << std::setw(4) << i << " > " << std::flush;
								std::cout << "_retcode is 0, the local recv buffer is " << _localRecvBuffers[i].size() << " long, we need a body." << std::endl;
								std::cout << std::setw(4) << i << " > " << std::flush;
								std::cout << "content-length for this one is supposed to be " << _perConnArr[i]->getContLen() << "..." << std::endl;
								if (_perConnArr[i]->getContLen() < _localRecvBuffers[i].size())
								{
									_perConnArr[i]->setNeedsBody(false);
									_debugMsgI(i, "Content size is too big, sending a 400");
									ResponseGenerator	responseObject("400 Bad Request");

									if (_sbufSize < responseObject.getSize())
									{
										_localSendString = responseObject.getText().substr(0, _sbufSize);
										send(socks[i].fd, _localSendString.c_str(), _sbufSize, 0);

										socks[i].events == POLLOUT;
										_perConnArr[i].setSendStr(responseObject.getText());
										_perConnArr[i].eraseSendStr(0, _sbufSize);
										_perConnArr[i].setStillResponding = true;

										_debugMsgI(i, "all the response:");
										std::cout << responseObject.getText() << std::flush;
										_debugMsgI(i, "the chunk that was sent:");
										std::cout << _localSendString << std::flush;
									}
									else
									{
										send(socks[i].fd, responseObject.getText().c_str(), responseObject.getSize(), 0);
										_perConnArr[i].setStillResponding = false;
										_debugMsgI(i, "sent in one go:");
										std::cout << responseObject.getText() << std::flush;
									}
									_perConnArr[i]->setTimeStarted(time(NULL));
									_debugMsgI(i, "reset starting time");

									_localRecvBuffers[i].clear();
									if ((!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted())
											&& (!_perConnArr[i].getStillResponding()))
									{
										_purgeOneConnection(i, &(socks[i].fd));
									}
								}
								else
								{
									_perConnArr[i]->setNeedsBody(false);
									try
									{
										// the try catch is for having a normal response generator, which will be able to throw errors like 502
										// trim the body here
//										someMythicalStringThatWillHoldTheBodyForUseByServer = _localRecvBuffers[i].substr(0, _perConnArr[i]->getContLen());
										ResponseGenerator	responseObject(200);

										if (_sbufSize < responseObject.getSize())
										{
											_localSendString = responseObject.getText().substr(0, _sbufSize);
											send(socks[i].fd, _localSendString.c_str(), _sbufSize, 0);

											socks[i].events == POLLOUT;
											_perConnArr[i].setSendStr(responseObject.getText());
											_perConnArr[i].eraseSendStr(0, _sbufSize);
											_perConnArr[i].setStillResponding = true;

											_debugMsgI(i, "all the response:");
											std::cout << responseObject.getText() << std::flush;
											_debugMsgI(i, "the chunk that was sent:");
											std::cout << _localSendString << std::flush;
										}
										else
										{
											send(socks[i].fd, responseObject.getText().c_str(), responseObject.getSize(), 0);
											_perConnArr[i].setStillResponding = false;
											_debugMsgI(i, "sent in one go:");
											std::cout << responseObject.getText() << std::flush;
										}
										_perConnArr[i]->setTimeStarted(time(NULL));
										_debugMsgI(i, "reset starting time");

										_localRecvBuffers[i].clear();
									}
									catch (std::exception & e)
									{
										_debugMsgI(i, e.what());
										ResponseGenerator	responseObject(e.what());
										if (_sbufSize < responseObject.getSize())
										{
											_localSendString = responseObject.getText().substr(0, _sbufSize);
											send(socks[i].fd, _localSendString.c_str(), _sbufSize, 0);

											socks[i].events == POLLOUT;
											_perConnArr[i].setSendStr(responseObject.getText());
											_perConnArr[i].eraseSendStr(0, _sbufSize);
											_perConnArr[i].setStillResponding = true;

											_debugMsgI(i, "all the response:");
											std::cout << responseObject.getText() << std::flush;
											_debugMsgI(i, "the chunk that was sent:");
											std::cout << _localSendString << std::flush;
										}
										else
										{
											send(socks[i].fd, responseObject.getText().c_str(), responseObject.getSize(), 0);
											_perConnArr[i].setStillResponding = false;
											_debugMsgI(i, "sent in one go:");
											std::cout << responseObject.getText() << std::flush;
										}
										_perConnArr[i]->setTimeStarted(time(NULL));
										_debugMsgI(i, "reset starting time");

										_localRecvBuffers[i].clear();
									}
								}
							} /* okay, it didn't ask for a body... at least yet. maybe it's a head? */
							else if (_localRecvBuffers[i].find(CRLFCRLF) != std::string::npos ||
									_localRecvBuffers[i].find(LFCRLF) != std::string::npos ||
									_localRecvBuffers[i].find(CRLFLF) != std::string::npos ||
									_localRecvBuffers[i].find(LFLF) != std::string::npos)
							{
								_onHeadLocated(i, &(socks[i].fd));
							}
							else
							{
								// ok, it's not a head, there's nothing to read, and we're just there. close if needed.
								_debugMsgI(i, "Is that some sort of a joke?");
								_debugMsgI(i, "No double-endline, AND the buff isn't zero... But nothing to read.");
								_debugMsgI(i, "Timeout-checking...");
								if (!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted())
								{
									std::cout << "       yup, close it." << std::endl;
									_localRecvBuffers[i].clear();
									_purgeOneConnection(i, &(socks[i].fd));
								}
							}
						}
						else
						{
							_debugMsgI(i, "Is that some sort of a joke?");
							_debugMsgI(i, "Nothing in local buf, the read is 0, how tf did we even get polled????");
							_purgeOneConnection(i, &(socks[i].fd));
						}
					}
					else if (_retCode > 0)
					{
						// every successful recv we reset the timer
						_perConnArr[i]->setTimeStarted(time(NULL));
						_debugMsgI(i, "reset starting time");
						// TODO: bare CR to SP replace
						// (?)
						// https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-4
						std::cout << std::setw(4) << i << " > " << std::flush;
						std::cout << "Message on " << i << " received (maybe) partially, " << _retCode << " bytes, RBUF (w/o \\0) is " << _rbufSize << "." << std::endl;
						_debugMsgI(i, "received:");
						buf[_retCode] = 0;
						std::cout << buf << std::endl;
						_localRecvBuffers[i] += std::string(buf);

						_debugMsgI(i, "Checking for a double line-break (any combo of LF and CRLF), or maybe we already need a body...");
						if (_perConnArr[i]->getNeedsBody())
						{
							if (_localRecvBuffers[i].size() > _perConnArr[i]->getContLen())
							{
								_perConnArr[i]->setNeedsBody(false);
								_debugMsgI(i, "Content size is too big, sending a 400");
								ResponseGenerator	responseObject("400 Bad Request");
								if (_sbufSize < responseObject.getSize())
								{
									_localSendString = responseObject.getText().substr(0, _sbufSize);
									send(socks[i].fd, _localSendString.c_str(), _sbufSize, 0);

									socks[i].events == POLLOUT;
									_perConnArr[i].setSendStr(responseObject.getText());
									_perConnArr[i].eraseSendStr(0, _sbufSize);
									_perConnArr[i].setStillResponding = true;

									_debugMsgI(i, "all the response:");
									std::cout << responseObject.getText() << std::flush;
									_debugMsgI(i, "the chunk that was sent:");
									std::cout << _localSendString << std::flush;
								}
								else
								{
									send(socks[i].fd, responseObject.getText().c_str(), responseObject.getSize(), 0);
									_perConnArr[i].setStillResponding = false;
									_debugMsgI(i, "sent in one go:");
									std::cout << responseObject.getText() << std::flush;
								}
								_perConnArr[i]->setTimeStarted(time(NULL));
								_debugMsgI(i, "reset starting time");

								_localRecvBuffers[i].clear();
								if ((!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted())
										&& (!_perConnArr[i].getStillResponding()))
								{
									_purgeOneConnection(i, &(socks[i].fd));
								}
							}
							else if (_localRecvBuffers[i].size() == _perConnArr[i]->getContLen())
							{
								// TODO write the file or something idk.
								// TODO maybe close the connection if the keepalive is off type deal?
								_perConnArr[i]->setNeedsBody(false);
								try
								{
									// the try catch is for having a normal response generator, which will be able to throw errors like 502
									ResponseGenerator	responseObject(200);

									if (_sbufSize < responseObject.getSize())
									{
										_localSendString = responseObject.getText().substr(0, _sbufSize);
										send(socks[i].fd, _localSendString.c_str(), _sbufSize, 0);

										socks[i].events == POLLOUT;
										_perConnArr[i].setSendStr(responseObject.getText());
										_perConnArr[i].eraseSendStr(0, _sbufSize);
										_perConnArr[i].setStillResponding = true;

										_debugMsgI(i, "all the response:");
										std::cout << responseObject.getText() << std::flush;
										_debugMsgI(i, "the chunk that was sent:");
										std::cout << _localSendString << std::flush;
									}
									else
									{
										send(socks[i].fd, responseObject.getText().c_str(), responseObject.getSize(), 0);
										_perConnArr[i].setStillResponding = false;
										_debugMsgI(i, "sent in one go:");
										std::cout << responseObject.getText() << std::flush;
									}
									_perConnArr[i]->setTimeStarted(time(NULL));
									_debugMsgI(i, "reset starting time");

									_localRecvBuffers[i].clear();
								}
								catch (std::exception & e)
								{
									_debugMsgI(i, e.what());
									ResponseGenerator	responseObject(e.what());

									if (_sbufSize < responseObject.getSize())
									{
										_localSendString = responseObject.getText().substr(0, _sbufSize);
										send(socks[i].fd, _localSendString.c_str(), _sbufSize, 0);

										socks[i].events == POLLOUT;
										_perConnArr[i].setSendStr(responseObject.getText());
										_perConnArr[i].eraseSendStr(0, _sbufSize);
										_perConnArr[i].setStillResponding = true;

										_debugMsgI(i, "all the response:");
										std::cout << responseObject.getText() << std::flush;
										_debugMsgI(i, "the chunk that was sent:");
										std::cout << _localSendString << std::flush;
									}
									else
									{
										send(socks[i].fd, responseObject.getText().c_str(), responseObject.getSize(), 0);
										_perConnArr[i].setStillResponding = false;
										_debugMsgI(i, "sent in one go:");
										std::cout << responseObject.getText() << std::flush;
									}
									_perConnArr[i]->setTimeStarted(time(NULL));
									_debugMsgI(i, "reset starting time");

									_localRecvBuffers[i].clear();
								}
							}
							else
							{
								_debugMsgI(i, "Content smaller than expected... continue reading thru the next cycle!!!");
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
							_debugMsgI(i, "Continue reading thru the next cycle!!!");
						}
					}
				}
			} /* else if POLLIN */
			else if ((socks[i].revents & POLLOUT) == POLLOUT)
			{
				if (_sbufSize < _perConnArr[i].getSendStr().size())
				{
					_localSendString = _perConnArr[i].getSendStr().substr(0, _sbufSize);
					send(socks[i].fd, _localSendString.c_str(), _sbufSize, 0);

					_perConnArr[i].eraseSendStr(0, _sbufSize);

					_debugMsgI(i, "the chunk that was sent:");
					std::cout << _localSendString << std::flush;
				}
				else
				{
					send(socks[i].fd, _perConnArr[i].getSendStr().c_str(), _perConnArr[i].getSendStr.size(), 0);
					_perConnArr[i].setStillResponding = false;
					_debugMsgI(i, "sent in one go or remaints:");
					std::cout << _perConnArr[i].getSendStr() << std::flush;
				}

				_perConnArr[i]->setTimeStarted(time(NULL));
				_debugMsgI(i, "reset starting time");
				// close/wipe check? TODO
				// TODO wait, also, set the poll enum to pollin again
			} /* else if POLLOUT */
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
			for (int k = _socksN; k < _blogSize; k++)
			{
				_localRecvBuffers[k].clear();
				if (_perConnArr[k] != NULL)
				{
//					delete _perConnArr[k];
					// wait, what if deleting messes up the whole array compression thing!!!!!!!!!!!!!!!!!!!
					_perConnArr[k] = NULL;
				}
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
