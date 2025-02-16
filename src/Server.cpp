#include "../inc/Server.hpp"

Server::Server(int argc, char** argv)
{
	try {
		if (argc == 1)
			_config = new ServerConfig();
		else if (argc == 2)
			_config = new ServerConfig(argv[1]);
	} catch (std::exception &err)
	{
		std::cout << err.what() << std::endl;
	}
	
	_rbufSize = 4096;
	_sbufSize = 4096;
	//_rbufSize = 2;
	//_sbufSize = 2;
	//_blogSize = 4096;
	//_connsAmt = 4096;
	_blogSize = 4096;
	_connsAmt = 512;
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

int	Server::_checkAvailFdI(pollfd *socks) const
{
	int	ret = -1;
	int	i = _lstnN;
	while (i < _connsAmt)
	{
		if (socks[i].fd == -1)
		{
			ret = i;
			break ;
		}
		i++;
	}
	return (ret);
}

void	Server::_purgeOneConnection(int i, pollfd *socks)
{
	_localRecvBuffers[i].clear();
	close(socks[i].fd);
	socks[i].fd = -1;

	if (_perConnArr[i] != NULL)
	{
		_tempFdI = i + _connsAmt;
		if (socks[_tempFdI].fd != -1)
		{
			close(socks[_tempFdI].fd);
			socks[_tempFdI].fd = -1;
			// TODO
			_debugMsgI(i, "file -1'd; don't forget to unlink/remove it if it's from post and you need it gone");
		}

		delete _perConnArr[i];
		_perConnArr[i] = NULL;
	}
}

void	Server::_firstTimeSender(ResponseGenerator *rO, pollfd *socks, int i, bool clearLRB, bool purgeC)
{
	// in case of has file being true, the size would be just of the head
	if (static_cast<size_t>(_sbufSize) < rO->getSize())
	{
		_localSendString = rO->getText().substr(0, _sbufSize);
		send(socks[i].fd, _localSendString.c_str(), _sbufSize, 0);

		socks[i].events = POLLOUT;
		_perConnArr[i]->setSendStr(rO->getText());
		_perConnArr[i]->eraseSendStr(0, _sbufSize);
		_perConnArr[i]->setStillResponding(true);

		_debugMsgI(i, "all the response:");
		std::cout << rO->getText() << std::flush;
		_debugMsgI(i, "the chunk that was sent:");
		std::cout << _localSendString << std::endl;
	}
	else
	{
		send(socks[i].fd, rO->getText().c_str(), rO->getSize(), 0);
		_perConnArr[i]->setStillResponding(false);
		_debugMsgI(i, "sent in one go:");
		std::cout << rO->getText() << std::endl;
		if (_perConnArr[i]->getHasFile())
		{
			socks[i].events = POLLOUT;
			_perConnArr[i]->setSendingFile(true);
		}
		else
		{
			socks[i].events = POLLIN;
		}
	}
	_perConnArr[i]->setTimeStarted(time(NULL));
	_debugMsgI(i, "reset starting time");

	if (clearLRB)
	{
		_localRecvBuffers[i].clear();
	}

	if (purgeC)
	{
		if ((!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted())
				&& (!_perConnArr[i]->getStillResponding()) && (!_perConnArr[i]->getSendingFile()) && (!_perConnArr[i]->getWritingFile()))
		{
			// maybe, getstillresponding should be removed from this if. we could spend a long time collecting the answer, and we should drop then. maybe.
			_purgeOneConnection(i, socks);
			_debugMsgI(i, "connection purged from firsttime");
			std::cout << "fd must be -1: " << socks[i].fd << std::endl;
		}
	}
}

void	Server::_onHeadLocated(int i, pollfd *socks)
{
	_debugMsgI(i, "Head located. Stop reading for a moment");
	_debugMsgI(i, "Total msg:");
	std::cout << _localRecvBuffers[i] << std::endl;
	// in case RHP fails; keep-alive is the default for http 1.1
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
			_debugMsgI(i, "POST RO generation started");
			ResponseGenerator	responseObject(req);
			_debugMsgI(i, "POST RO generated successfully");
			_perConnArr[i]->setNeedsBody(true);
			_perConnArr[i]->setContLen(req.getContLen());
			_eraseDoubleNlInLocalRecvBuffer(i);
			_localFWriteBuffers[i] = _localRecvBuffers[i];

			_debugMsgI(i, "local file writing buffer filled with " + _localFWriteBuffers[i]);

			_perConnArr[i]->setWritingFile(true);
			_tempFdI = i + _connsAmt;
			socks[_tempFdI].fd = responseObject.getFd();
			socks[_tempFdI].events = POLLOUT;
			// FIXME clean me broooo vvvvvvvvvvvvvvv we don't need the fd variable no more? cuz like it's the socks[i + _connsAmt], always
			_perConnArr[i]->setFd(responseObject.getFd());
			// we don't send stuff just yet. 201 created should probably be sent when we're done reading, right?
			// then, we save the contents of ro to sendstr i think.
			_perConnArr[i]->setSendStr(responseObject.getText());
		}
		else if (req.getMethod() == "GET")
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
			_debugMsgI(i, "GET RO generation started");
			ResponseGenerator	responseObject(req);
			_debugMsgI(i, "GET RO generated successfully");
			// FIXME we don't need this if if it's always true after a non-throwing response object generation
			if (responseObject.getHasFile())
			{
				_tempFdI = i + _connsAmt;
				_perConnArr[i]->setHasFile(true);
				socks[_tempFdI].fd = responseObject.getFd();
				socks[_tempFdI].events = POLLIN;
				_perConnArr[i]->setFd(responseObject.getFd());
				_perConnArr[i]->setRemainingFileSize(responseObject.getFSize());
			}
			_firstTimeSender(&responseObject, socks, i, false, true);

			_eraseDoubleNlInLocalRecvBuffer(i);
		}
		else
		{
			// this is DELETE.... obviously, TODO. not implemented yet at all
			_perConnArr[i]->setNeedsBody(false);
			ResponseGenerator	responseObject(200);

			_firstTimeSender(&responseObject, socks, i, false, true);

			_eraseDoubleNlInLocalRecvBuffer(i);
		}
	}
	catch (std::exception & e)
	{
		// TODO specific server errors (if it's a malloc error, just send a 500 or something. check by making all our server errors of the same sub-class or something.)
		_debugMsgI(i, "caught something");
		_perConnArr[i]->setHasFile(false);
		_perConnArr[i]->setSendingFile(false);
		_perConnArr[i]->setNeedsBody(false);
		_debugMsgI(i, e.what());
		ResponseGenerator	responseObject(e.what());

		_firstTimeSender(&responseObject, socks, i, true, true);
	}
}

void	Server::run(void)
{
	std::vector<int>	lPorts = _config->getConfig().back().ports;
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
	_lstnN = _listenSocks.size();
	for (int i = 0; i < _lstnN; i++)
	{
		socks[i].fd = _listenSocks[i];
		socks[i].events = POLLIN;
	}

	// _timeout is measured in ms
	_timeout = 1 * 1000;
	// newconnect's -2 is its init value while -1 indicates a fail
	_newConnect = -2;
	char	buf[_rbufSize + 1];
	char	fileToReadBuf[_sbufSize + 1];
	// this will store what we read because we shall be able to read in multiple passes before closing the connection.
	_localRecvBuffers = std::vector<std::string>(_connsAmt, "");
	// this will store post request strings for them to be written chunk by chunk into the files via poll
	_localFWriteBuffers = std::vector<std::string>(_connsAmt, "");
	_fWCounts = std::vector<size_t>(_connsAmt, 0);

	std::cout << "Alright, starting. Ports: " << *(lPorts.begin()) << ".." << *(lPorts.end() - 1) << std::endl;
	_running = true;
	time_t	curTime;
	while (_running)
	{
		_retCode = poll(socks, _connsAmt * 2, _timeout);
		curTime = time(NULL);
		if (_retCode < 0)
		{
			throw pollError();
		}
		else if (_retCode == 0)
		{
			for (int i = _lstnN; i < _connsAmt; i++)
			{
				if (_perConnArr[i] != NULL)
				{
					if ((!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < curTime - _perConnArr[i]->getTimeStarted())
							&& (!_perConnArr[i]->getStillResponding()) && (!_perConnArr[i]->getWritingFile()))
					{
						_purgeOneConnection(i, socks);
					}
				}
			}
		}
		std::cout << "Just poll'd, ret number is " << _retCode << std::endl;

		// go thru all the socks that could have possibly been affected
		// see if they've been affected
		for (int i = 0; i < _connsAmt; i++)
		{
			if (socks[i].fd == -1)
				continue ;
			if (_perConnArr[i] != NULL)
			{
				_debugMsgTimeI(i, curTime);
				if ((!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < curTime - _perConnArr[i]->getTimeStarted())
						&& (!_perConnArr[i]->getStillResponding()) && (!_perConnArr[i]->getSendingFile()) && (!_perConnArr[i]->getWritingFile()))
				{
					_purgeOneConnection(i, socks);
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
					// XXX ?
				}
				_debugMsgI(i, "got an err/hup/val");
				_purgeOneConnection(i, socks);
			}
			else if ((socks[i].revents & POLLIN) == POLLIN)
			{
				if (i < _lstnN)
				{
					// wow is that the listening fd? our sock? yes it is!
					_connectHereIndex = _checkAvailFdI(socks);
					if (_connectHereIndex != -1)
					{
						_newConnect = accept(_listenSocks[i], NULL, NULL);
//						_newConnect = accept(socks[i].fd, NULL, NULL);
						while (_newConnect > 0)
						{
//							_debugMsgI(i, "accepted a connect");
							socks[_connectHereIndex].fd = _newConnect;
							socks[_connectHereIndex].events = POLLIN;
							_perConnArr[_connectHereIndex] = new Connect;
//							_debugMsgI(_connectHereIndex, "<-- new connect is situated here");
							_connectHereIndex = _checkAvailFdI(socks);
							if (_connectHereIndex != -1)
							{
								_newConnect = accept(_listenSocks[i], NULL, NULL);
							}
							else
							{
								break ;
							}
						}
					}
				}
				else
				{
//					std::cout << "Descriptior " << socks[i].fd << " at pos " << i << " readable" << std::endl;
					std::memset(buf, 0, sizeof (buf));
					_retCode = recv(socks[i].fd, buf, _rbufSize, 0);
					if (_retCode < 0)
					{
						// to crash or not to crash? XXX to think
//						throw readError();
						_purgeOneConnection(i, socks);
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
								std::cout << "_retCode is 0, the local recv buffer is " << _localRecvBuffers[i].size() << " long, we need a body." << std::endl;
								std::cout << std::setw(4) << i << " > " << std::flush;
								std::cout << "content-length for this one is supposed to be " << _perConnArr[i]->getContLen() << "..." << std::endl;
								if (_perConnArr[i]->getContLen() < _localRecvBuffers[i].size())
								{
									_perConnArr[i]->setNeedsBody(false);
									_debugMsgI(i, "Content size is too big, sending a 400");
									ResponseGenerator	responseObject("400 Bad Request");

									_firstTimeSender(&responseObject, socks, i, true, true);
								}
								else
								{
									_perConnArr[i]->setNeedsBody(false);
									// same as with retcode > 0, do nothing til the file is done.
//									try
//									{
//										// the try catch is for having a normal response generator, which will be able to throw errors like 502
//										// trim the body here
////										someMythicalStringThatWillHoldTheBodyForUseByServer = _localRecvBuffers[i].substr(0, _perConnArr[i]->getContLen());
//										ResponseGenerator	responseObject(200);
//
//										_firstTimeSender(&responseObject, socks, i, true, true);
//									}
//									catch (std::exception & e)
//									{
//										_debugMsgI(i, e.what());
//										ResponseGenerator	responseObject(e.what());
//
//										_firstTimeSender(&responseObject, socks, i, true, true);
//									}
								}
							} /* okay, it didn't ask for a body... at least yet. maybe it's a head? */
							else if (_localRecvBuffers[i].find(CRLFCRLF) != std::string::npos ||
									_localRecvBuffers[i].find(LFCRLF) != std::string::npos ||
									_localRecvBuffers[i].find(CRLFLF) != std::string::npos ||
									_localRecvBuffers[i].find(LFLF) != std::string::npos)
							{
								_onHeadLocated(i, socks);
							}
							else
							{
								// ok, it's not a head, there's nothing to read, and we're just there. close if needed.
								_debugMsgI(i, "Is that some sort of a joke?");
								_debugMsgI(i, "No double-endline, AND the buff isn't zero... But nothing to read.");
								_debugMsgI(i, "Timeout-checking...");
								// XXX file cond?....
								if (!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted())
								{
									std::cout << "       yup, close it." << std::endl;
									_purgeOneConnection(i, socks);
								}
							}
						}
						else
						{
							_debugMsgI(i, "Is that some sort of a joke?");
							_debugMsgI(i, "Nothing in local buf, the read is 0, how tf did we even get polled????");
							_purgeOneConnection(i, socks);
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
						std::cout << "Message received (maybe) partially, " << _retCode << " bytes, RBUF (w/o \\0) is " << _rbufSize << "." << std::endl;
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
								_firstTimeSender(&responseObject, socks, i, true, true);
								// delete the file that's too big TODO
							}
							else if (_localRecvBuffers[i].size() == _perConnArr[i]->getContLen())
							{
								_localFWriteBuffers[i] += std::string(buf);
								_perConnArr[i]->setNeedsBody(false);
								// we don't try to reply here. we only try to reply if we've finished writing the file to the disk.
//								try
//								{
//									// the try catch is for having a normal response generator, which will be able to throw errors like 502
//									ResponseGenerator	responseObject(200);
//									_firstTimeSender(&responseObject, socks, i, true, true);
//								}
//								catch (std::exception & e)
//								{
//									_debugMsgI(i, e.what());
//									ResponseGenerator	responseObject(e.what());
//									_firstTimeSender(&responseObject, socks, i, true, true);
//								}
							}
							else
							{
								_localFWriteBuffers[i] += std::string(buf);
								_debugMsgI(i, "Content smaller than expected... continue reading thru the next cycle. Added to local fw buf.");
							}
						}
						else if (_localRecvBuffers[i].find(CRLFCRLF) != std::string::npos ||
								_localRecvBuffers[i].find(LFCRLF) != std::string::npos ||
								_localRecvBuffers[i].find(CRLFLF) != std::string::npos ||
								_localRecvBuffers[i].find(LFLF) != std::string::npos)
						{
							_onHeadLocated(i, socks);
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
				// regular sends (not first sends)
				// step 1. are we sending a file rn? no? well, just send the string that's saved in the perconarr[i]
				if (!_perConnArr[i]->getSendingFile())
				{
					if (static_cast<size_t>(_sbufSize) < _perConnArr[i]->getSendStr().size())
					{
						_localSendString = _perConnArr[i]->getSendStr().substr(0, _sbufSize);
						send(socks[i].fd, _localSendString.c_str(), _sbufSize, 0);

						_perConnArr[i]->eraseSendStr(0, _sbufSize);

						_debugMsgI(i, "the chunk that was sent:");
						std::cout << _localSendString << std::endl;
					}
					else
					{
						send(socks[i].fd, _perConnArr[i]->getSendStr().c_str(), _perConnArr[i]->getSendStr().size(), 0);
						_perConnArr[i]->setStillResponding(false);
						_debugMsgI(i, "sent in one go or remaints:");
						std::cout << _perConnArr[i]->getSendStr() << std::endl;
						if (_perConnArr[i]->getHasFile())
						{
							_debugMsgI(i, "switching to filesending mode");
							_debugMsgI(socks[i + _connsAmt].fd, "<- file's fd");
							if (socks[i + _connsAmt].events == POLLIN)
								_debugMsgI(i, "pollin on the file is set");
							_perConnArr[i]->setSendingFile(true);
						}
						else
						{
							socks[i].events = POLLIN;
						}
					}
				}
				// oh, we have a file to send? ok. we need to read it, in the same chunk by chunk manner.
				else
				{
					_tempFdI = i + _connsAmt;
					// after finding where the file fd is in the poll's list, we check for its revents. irl, it's probably absolutely always ready
					// but here we have a subject to abide by :P
					if (((socks[_tempFdI].revents & POLLERR) == POLLERR)
							|| ((socks[_tempFdI].revents & POLLHUP) == POLLHUP)
							|| ((socks[_tempFdI].revents & POLLNVAL) == POLLNVAL))
					{
						_debugMsgI(i, "while processing the socket,..");
						_debugMsgI(_tempFdI, "<- its associated file errored.");
					}
					else if ((socks[_tempFdI].revents & POLLIN) == POLLIN)
					{
						std::memset(fileToReadBuf, 0, sizeof (fileToReadBuf));
						_retCode = read(socks[_tempFdI].fd, fileToReadBuf, _sbufSize);
						if (_retCode < 0)
						{
							// file reading error. this is weird; we should've caught it upon opening the file,
							// but maybe we're reading a 2gb file and the admin deleted it while we were in the middle
							// thus, this needs appropriate handling FIXME
							_debugMsgI(i, "retcode on file is < 0. crashing for lols.");
							return ;
						}
						else if (_retCode == 0)
						{
							// we somehow had something from poll but showed up with 0 bytes upon actual reading.
							// mathematically unlikely, as bob page perfectly put it in the hit videogame deus ex from the year 2000.
							_debugMsgI(i, "ABSOLUTELY done reading from the asked file. close it up.");
							close(socks[_tempFdI].fd);
							socks[_tempFdI].fd = -1;

							_perConnArr[i]->setHasFile(false);
							_perConnArr[i]->setSendingFile(false);
							_perConnArr[i]->setStillResponding(false);
							socks[i].events = POLLIN;
						}
						else if (_perConnArr[i]->getRemainingFileSize() == _retCode)
						{
							// the amount of file left is exactly the amount that was read. ggs
							_debugMsgI(i, "the intended way of finishing reading a file reached.");
							fileToReadBuf[_retCode] = 0;
							_localSendString = std::string(fileToReadBuf);
							send(socks[i].fd, _localSendString.c_str(), _retCode, 0);

							// what's the purpose...
//							_perConnArr[i]->diminishRemainingFileSize(_sbufSize);

							_debugMsgI(i, "the last f-chunk that was sent:");
							std::cout << _localSendString << std::endl;

							close(socks[_tempFdI].fd);
							socks[_tempFdI].fd = -1;

							_perConnArr[i]->setHasFile(false);
							_perConnArr[i]->setSendingFile(false);
							_perConnArr[i]->setStillResponding(false);
							socks[i].events = POLLIN;
						}
						else
						{
							// the "regular". we need to substract the sbuf size from the counter of remaining filesize and send the chunk.
							fileToReadBuf[_retCode] = 0;
							_localSendString = std::string(fileToReadBuf);
							send(socks[i].fd, _localSendString.c_str(), _sbufSize, 0);

							_perConnArr[i]->diminishRemainingFileSize(_sbufSize);

							_debugMsgI(i, "the f-chunk that was sent:");
							std::cout << _localSendString << std::endl;
						}
					}
					else
					{
						// file not hit by revents. mathematically unlikely.
						_debugMsgI(i, "file not ready yet");
						_debugMsgI(_tempFdI, "<- file");
						_debugMsgI(socks[_tempFdI].fd, "<- file's fd");
						std::cout << socks[_tempFdI].revents << std::endl;
						continue ;
					}
				}

				_perConnArr[i]->setTimeStarted(time(NULL));
				_debugMsgI(i, "reset starting time");

				//something to ponder XXX
//				_localRecvBuffers[i].clear();
				if ((!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted())
						&& (!_perConnArr[i]->getStillResponding()) && (!_perConnArr[i]->getSendingFile()))
				{
					// maybe, getstillresponding should be removed from this if. we could spend a long time collecting the answer, and we should drop then. maybe.
					_purgeOneConnection(i, socks);
					_debugMsgI(i, "connection purged in pollout");
				}
			} /* else if POLLOUT */
		} /* for to iterate thru socks upon poll's return */
		// time to iterate thru files!
		for (int i = _connsAmt; i < _connsAmt * 2; i++)
		{
			if (socks[i].fd == -1 || ((socks[i].events & POLLIN) == POLLIN))
			{
				continue ;
			}
			if (socks[i].revents == 0)
			{
				std::cout << "revents on " << i << " is 0" << std::endl;
				if ((!_perConnArr[i - _connsAmt]->getKeepAlive() || _perConnArr[i - _connsAmt]->getKaTimeout() < time(NULL) - _perConnArr[i - _connsAmt]->getTimeStarted()))
				{
					// maybe, getstillresponding should be removed from this if. we could spend a long time collecting the answer, and we should drop then. maybe.
					_purgeOneConnection(i - _connsAmt, socks);
					_debugMsgI(i, "connection timeouted in filewriting");
				}
				continue ;
			}
			else if (((socks[i].revents & POLLERR) == POLLERR) || ((socks[i].revents & POLLHUP) == POLLHUP) || ((socks[i].revents & POLLNVAL) == POLLNVAL))
			{
				_debugMsgI(i, "got an err/hup/val");
				_purgeOneConnection(i - _connsAmt, socks);
				continue ;
			}
			else if ((socks[i].revents & POLLOUT) == POLLOUT)
			{
				if (static_cast<size_t>(_rbufSize) < _localFWriteBuffers[i - _connsAmt].size())
				{
					write(socks[i].fd, _localFWriteBuffers[i - _connsAmt].c_str(), _rbufSize);
					_debugMsgI(i, "wrote an rbuf-sized chunk");
					_debugMsgI(i, _localFWriteBuffers[i - _connsAmt].substr(0, _rbufSize));
					_fWCounts[i - _connsAmt] += _rbufSize;
					_localFWriteBuffers[i - _connsAmt].erase(0, _rbufSize);
					_debugMsgI(i, "remains after erasing:");
					_debugMsgI(i, _localFWriteBuffers[i - _connsAmt]);
				}
				else
				{
					write(socks[i].fd, _localFWriteBuffers[i - _connsAmt].c_str(), _localFWriteBuffers[i - _connsAmt].size());
					_debugMsgI(i, "wrote a less than rbuf sized chunk");
					_debugMsgI(i, _localFWriteBuffers[i - _connsAmt]);
					_fWCounts[i - _connsAmt] += _localFWriteBuffers[i - _connsAmt].size();
					_localFWriteBuffers[i - _connsAmt].clear();
				}
				_debugMsgI(i, "total fwcount is..");
				std::cout << _fWCounts[i - _connsAmt] << std::endl;
				_perConnArr[i - _connsAmt]->setTimeStarted(time(NULL));
				_debugMsgI(i, "reset starting time");
			}
			if (_fWCounts[i - _connsAmt] == _perConnArr[i - _connsAmt]->getContLen())
			{
//				_debugMsgI(i, "closing the file");
//				_debugMsgI(_fWCounts[i - _connsAmt], "<- fdcounts for the file was");
//				_debugMsgI(_perConnArr[i - _connsAmt]->getContLen(), "<- contlen for the file was");
				// done. close the file
				close(socks[i].fd);
				socks[i].fd = -1;
				// must send the 201 created now
				socks[i - _connsAmt].events = POLLOUT;
				_perConnArr[i - _connsAmt]->setWritingFile(false);
				_perConnArr[i - _connsAmt]->setNeedsBody(false);
				_perConnArr[i - _connsAmt]->setStillResponding(true);
			}
			else if (_fWCounts[i - _connsAmt] > _perConnArr[i - _connsAmt]->getContLen())
			{
				// XXX TODO FIXME
				_debugMsgI(i, "too much");
				return ;
			}
			else if ((!_perConnArr[i - _connsAmt]->getKeepAlive() || _perConnArr[i - _connsAmt]->getKaTimeout() < time(NULL) - _perConnArr[i - _connsAmt]->getTimeStarted()))
			{
				// maybe, getstillresponding should be removed from this if. we could spend a long time collecting the answer, and we should drop then. maybe.
				_purgeOneConnection(i - _connsAmt, socks);
				_debugMsgI(i, "connection timeouted in filewriting");
			}
		} /* for files in socks */
		std::cout << "                one cycle done!" << std::endl;
	} /* while (_running) */
}

Server::~Server()
{
	delete _config;
}
