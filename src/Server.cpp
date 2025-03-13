#include "../inc/Server.hpp"

bool	killMe = false;

Server::Server()
{
}

unsigned long	Server::_strIpToUL(std::string ip) const
{
	unsigned long	res = 0;
	int	helper;
	int	index = 3;
	size_t	point = ip.find('.');
	while (point != std::string::npos)
	{
		std::istringstream	iss(ip.substr(0, point));
		iss >> helper;
		res += static_cast<unsigned long>(helper);
		res <<= 8;
		index--;
		ip.erase(0, point + 1);
		point = ip.find('.');
	}
	std::istringstream	iss(ip);
	iss >> helper;
	res += helper;
	return (res);
}

void	Server::_parseEnv(char **env)
{
	for (int i = 0; env[i]; i++)
		_env.push_back(env[i]);
}

void	_gracefulExit(int sig)
{
	(void)sig;
	std::cout << "Shutting down..." << std::endl;
	killMe = true;
}

Server::Server(int argc, char** argv, char **env)
{
	_rbufSize = 4096;
	_sbufSize = 4096;
	_blogSize = 4096;
	_connsAmt = CONNS_AMT;
	_parseEnv(env);
	if (argc == 1)
		_grandConfig = new ServerConfig();
	else if (argc == 2)
		_grandConfig = new ServerConfig(argv[1]);

	_perConnArr = std::vector<Connect * >(_connsAmt, NULL);
	_nls.push_back(CRLFCRLF);
	_nls.push_back(LFCRLF);
	_nls.push_back(CRLFLF);
	_nls.push_back(LFLF);
	signal(SIGINT, _gracefulExit);
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
#ifdef DEBUG_SERVER_MESSAGES
	std::cout << std::setw(4) << i << " > " << std::flush;
	std::cout << msg << std::endl;
#else
	(void)i;
	(void)msg;
#endif
}

void	Server::_debugMsgTimeI(int i, time_t curTime)
{
#ifdef DEBUG_SERVER_MESSAGES
	std::cout << std::setw(4) << i << " > " << std::flush;
	std::cout << "time started: " << _perConnArr[i]->getTimeStarted() << ", diff with now: " << curTime - _perConnArr[i]->getTimeStarted() << std::endl;
#else
	(void)i;
	(void)curTime;
#endif
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

int	Server::_checkAvailFdI(void) const
{
	int	ret = -1;
	int	i = _lstnN;
	while (i < _connsAmt)
	{
		if (_socks[i].fd == -1)
		{
			ret = i;
			break ;
		}
		i++;
	}
	return (ret);
}

void	Server::_purgeOneConnection(int i)
{
	if (i < _connsAmt)
	{
		_localRecvBuffers[i].clear();
		_localSendStrings[i].clear();
		_localFWriteBuffers[i].clear();
		_fWCounts[i] = 0;
		if (_perConnArr[i] != NULL)
		{
			_tempFdI = i + _connsAmt;
			if (_socks[_tempFdI].fd != -1)
			{
				close(_socks[_tempFdI].fd);
				_socks[_tempFdI].fd = -1;
//				_debugMsgI(i, "file -1'd; don't forget to unlink/remove it if it's from post and you need it gone");
			}
			_socks[_tempFdI].events = 0;
			_socks[_tempFdI].revents = 0;

			delete _perConnArr[i];
			_perConnArr[i] = NULL;
		}
	}
	if (_socks[i].fd != -1)
	{
		close(_socks[i].fd);
		_socks[i].fd = -1;
	}
	_socks[i].events = 0;
	_socks[i].revents = 0;

//	_debugMsgI(i, "thou gotst purg'd!");
}

void	Server::_responseObjectHasAFile(int i, ResponseGenerator *responseObject)
{
	_tempFdI = i + _connsAmt;
	_debugMsgI(i, "entered \"ro has a file\"");
	_perConnArr[i]->setHasFile(true);
	_socks[_tempFdI].fd = responseObject->getFd();
	_socks[_tempFdI].events = POLLIN;
	_perConnArr[i]->setFd(responseObject->getFd());
	_perConnArr[i]->setRemainingFileSize(responseObject->getFSize());
}

void	Server::_contentTooBigHandilng(int i)
{
	// erroneous request means file deletion.
	if (!_perConnArr[i]->getIsCgi())
	{
		remove(_perConnArr[i]->getRTarget().c_str());
	}
	_cleanAfterCatching(i);
	_debugMsgI(i, "Content size is too big, sending a 400");
	ResponseGenerator	responseObject("400 Bad Request", _perConnArr[i]->getServerContext());
	if (responseObject.getHasFile())
	{
		_responseObjectHasAFile(i, &responseObject);
	}

	_firstTimeSender(&responseObject, i, true, true);
}

void	Server::_cleanAfterCatching(int i)
{
	close(_perConnArr[i]->getFd());
	_socks[i + _connsAmt].fd = -1;
	_perConnArr[i]->setSendStr("");
	_perConnArr[i]->setHasFile(false);
	_perConnArr[i]->setSendingFile(false);
	_perConnArr[i]->setWritingFile(false);
	_perConnArr[i]->setNeedsBody(false);
}

void	Server::_cleanAfterNormalRead(int i)
{
	_tempFdI = i + _connsAmt;
	close(_socks[_tempFdI].fd);
	_socks[_tempFdI].fd = -1;

	_perConnArr[i]->setHasFile(false);
	_perConnArr[i]->setSendingFile(false);
	_perConnArr[i]->setStillResponding(false);
	_perConnArr[i]->setWritingFile(false);
	_socks[i].events = POLLIN;
	_localSendStrings[i].clear();
}

std::string	Server::_parseCgiStatus(char * fbuf)
{
	std::string	b(fbuf);
	size_t		stPos = b.find("Status: ");
	size_t		nlPos = b.find("\n", stPos);
	std::string	ret;
	if (stPos == std::string::npos)
	{
		ret = "200 OK";
	}
	else
	{
		ret = b.substr(stPos + 8, nlPos - stPos - 8);
	}
	return (ret);
}

void	Server::_firstTimeSender(ResponseGenerator *rO, int i, bool clearLRB, bool purgeC)
{
	_perConnArr[i]->setTimeStarted(time(NULL));
	if (_perConnArr[i]->getIsCgi())
	{
		_socks[i].events = POLLOUT;
		_perConnArr[i]->setStillResponding(true);
		_localSendStrings[i].clear();
		_localFWriteBuffers[i].clear();
		_perConnArr[i]->setCgiTimeStarted(time(NULL));
		_perConnArr[i]->setPid(rO->getPid());
	}
	else
	{
		// in case of has file being true, the size would be just of the head
		_localSendStrings[i] = std::string(rO->getText());

		_socks[i].events = POLLOUT;
		_perConnArr[i]->setSendStr(rO->getText());
		_perConnArr[i]->setStillResponding(true);
	}

	if (clearLRB)
	{
		_localRecvBuffers[i].clear();
	}

	// TODO please check if ever used.
	if (purgeC)
	{
		if ((!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted())
				&& (!_perConnArr[i]->getStillResponding()) && (!_perConnArr[i]->getSendingFile()) && (!_perConnArr[i]->getWritingFile()))
		{
			_purgeOneConnection(i);
		}
	}
}

void	Server::_onHeadLocated(int i)
{
//	std::cout << "head located: " << std::endl;
//	std::cout << _localRecvBuffers[i] << std::endl;
	try
	{
		RequestHeadParser		req(_localRecvBuffers[i], _perConnArr[i]->getServerContext());
		_perConnArr[i]->setKeepAlive(req.getKeepAlive());
		_perConnArr[i]->setKaTimeout(req.getKaTimeout());
		_perConnArr[i]->setIsCgi(req.getIsCgi());
		if (req.getMethod() == "POST")
		{
			if (_perConnArr[i]->getIsCgi())
			{
				// upon cgi: don't read and write to file. eliminate the header, collect ALL the body and THEN generate the response.
				_perConnArr[i]->setRTarget(req.getRTarget());
				_perConnArr[i]->setNeedsBody(true);
				_debugMsgI(req.getContLen(), "<- cont len");
				_perConnArr[i]->setContLen(req.getContLen());
				if (_perConnArr[i]->getContLen() > _perConnArr[i]->getServerContext().maxBodySize)
				{
					throw contentTooLarge();
				}
				_eraseDoubleNlInLocalRecvBuffer(i);
				if (_localRecvBuffers[i].size() == _perConnArr[i]->getContLen())
				{
					_debugMsgI(i, "cgi post hit inside of head located");
					_postReadingIsDone(i);
				}
				// local fw buffers now only serves the body collection function. we will not use it to write anything.
			}
			else
			{
				// this is for file deletion purposes in case of fail
				_perConnArr[i]->setRTarget(req.getRTarget());
				ResponseGenerator	responseObject(req, _perConnArr[i]->getServerContext(), _env);
				_perConnArr[i]->setNeedsBody(true);
				_perConnArr[i]->setContLen(req.getContLen());
				if (_perConnArr[i]->getContLen() > _perConnArr[i]->getServerContext().maxBodySize && !_perConnArr[i]->getIsCgi())
				{
					remove(_perConnArr[i]->getRTarget().c_str());
					throw contentTooLarge();
				}
				_eraseDoubleNlInLocalRecvBuffer(i);
				_localFWriteBuffers[i] = _localRecvBuffers[i];
				_localRecvBuffers[i].clear();

				_perConnArr[i]->setWritingFile(true);
				_tempFdI = i + _connsAmt;
				_socks[_tempFdI].fd = responseObject.getFd();
				_socks[_tempFdI].events = POLLOUT;
				_perConnArr[i]->setFd(responseObject.getFd());
				// this sets up a 201 response
				_perConnArr[i]->setSendStr(responseObject.getText());
			}
		}
		else if (req.getMethod() == "GET")
		{
			_perConnArr[i]->setNeedsBody(false);
			ResponseGenerator	responseObject(req, _perConnArr[i]->getServerContext(), _env);
			// this if is important, since we can return a dirlist instead
			if (responseObject.getHasFile())
			{
				_responseObjectHasAFile(i, &responseObject);
			}
			_firstTimeSender(&responseObject, i, false, true);

			_eraseDoubleNlInLocalRecvBuffer(i);
		}
		else /* DELETE */
		{
			_perConnArr[i]->setNeedsBody(false);
			ResponseGenerator	responseObject(req, _perConnArr[i]->getServerContext(), _env);

			_firstTimeSender(&responseObject, i, false, true);

			_eraseDoubleNlInLocalRecvBuffer(i);
		}
	}
	catch (pipeError & e)
	{
		for (int i = 0; i < _connsAmt * 2; i++)
		{
			_purgeOneConnection(i);
		}
		throw pipeError();
	}
	catch (execveError & e)
	{
		for (int i = 0; i < _connsAmt * 2; i++)
		{
			_purgeOneConnection(i);
		}
		throw execveError();
	}
	catch (std::exception & e)
	{
		// TODO parse keepalive from http in this
		_cleanAfterCatching(i);
		ResponseGenerator	responseObject(e.what(), _perConnArr[i]->getServerContext());
		if (responseObject.getHasFile())
		{
			_responseObjectHasAFile(i, &responseObject);
		}

		_firstTimeSender(&responseObject, i, true, true);
	}
}

void	Server::_postReadingIsDone(int i)
{
	_perConnArr[i]->setNeedsBody(false);
	if (_perConnArr[i]->getIsCgi())
	{
		try
		{
			ResponseGenerator	rO(_localRecvBuffers[i], _perConnArr[i]->getRTarget(), _env);
			_responseObjectHasAFile(i, &rO);
			_firstTimeSender(&rO, i, false, true);
		}
		catch (pipeError & e)
		{
			for (int i = 0; i < _connsAmt * 2; i++)
			{
				_purgeOneConnection(i);
			}
			throw pipeError();
		}
		catch (execveError & e)
		{
			for (int i = 0; i < _connsAmt * 2; i++)
			{
				_purgeOneConnection(i);
			}
			throw execveError();
		}
		catch (std::exception & e)
		{
			_cleanAfterCatching(i);
			ResponseGenerator	responseObject(e.what(), _perConnArr[i]->getServerContext());
			if (responseObject.getHasFile())
			{
				_responseObjectHasAFile(i, &responseObject);
			}

			_firstTimeSender(&responseObject, i, true, true);
		}
	}
}

void	Server::_acceptNewConnect(int i)
{
	_socks[_connectHereIndex].fd = _newConnect;
	_socks[_connectHereIndex].events = POLLIN;
	_socks[_connectHereIndex].revents = 0;
	_perConnArr[_connectHereIndex] = new Connect;
	_perConnArr[_connectHereIndex]->setServerContext(_perConnArr[i]->getServerContext());
	_localRecvBuffers[i].clear();
	_localSendStrings[i].clear();
	_localFWriteBuffers[i].clear();
	_fWCounts[i] = 0;
	_connectHereIndex = _checkAvailFdI();
}

void	Server::_serverRunSetupInit(void)
{
	// 1st for goes thru the servers, since each server has one ip adress
	// the perconnarr contexts for the listen sockets are set on a per-server basis. ports are saved for redirections etc.
	for (unsigned long servI = 0; servI < _grandConfig->getConfig().size(); servI++)
	{
		struct config_server_t	current_conf = _grandConfig->getConfig()[servI];
		_debugMsgI(servI, "IP registred: " + current_conf.host);
		// 2nd for goes thru its ports
		for (unsigned long portI = 0; portI < current_conf.ports.size(); portI++)
		{
			_debugMsgI(portI, "port registred:");
			_debugMsgI(current_conf.ports[portI], "<-");
			sockaddr_in	address;
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
			std::memset(&(address), 0, sizeof (address));
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = htonl(_strIpToUL(current_conf.host));
			address.sin_port = htons(current_conf.ports[portI]);
			// make a sock be of a specified address
			if (bind(_listenSock, (struct sockaddr *)&(address), sizeof(address)) < 0)
				throw bindError();

			// listen up!
			if (listen(_listenSock, _blogSize) < 0)
				throw listenError();

			_listenSocks.push_back(_listenSock);
			_perConnArr[_listenSocks.size() - 1] = new Connect;
			_perConnArr[_listenSocks.size() - 1]->setServerContext(current_conf);
			_perConnArr[_listenSocks.size() - 1]->setPortInUse(current_conf.ports[portI]);
		}
	}
	// _socks:
	// [0         ...      _lS.size) -- for listening sockets
	// [_lS.size  ...     _connsAmt) -- for accepted connections
	// [_connsAmt ... _connsAmt * 2) -- for files and cgi pipes
	for (int x = 0; x < _connsAmt * 2; x++)
	{
		_socks[x].fd = -1;
		_socks[x].events = 0;
		_socks[x].revents = 0;
	}

	_lstnN = _listenSocks.size();
	for (int i = 0; i < _lstnN; i++)
	{
		_socks[i].fd = _listenSocks[i];
		_socks[i].events = POLLIN;
	}

	// _timeout is measured in ms
	_timeout = 1 * 1000;
	// newconnect's -2 is its init value while -1 indicates a fail
	_newConnect = -2;
	_localRecvBuffers = std::vector<std::string>(_connsAmt, "");
	// this will store post request strings for them to be written chunk by chunk into the files via poll
	_localFWriteBuffers = std::vector<std::string>(_connsAmt, "");
	_localSendStrings = std::vector<std::string>(_connsAmt, "");
	_fWCounts = std::vector<size_t>(_connsAmt, 0);
}

void	Server::run(void)
{
    if (!_grandConfig->getConfig().size())
		return ;

	_serverRunSetupInit();
	char	buf[_rbufSize + 1];
	char	fileToReadBuf[_rbufSize + 1];

	_running = true;
	while (_running || !killMe)
	{
		_retCode = poll(_socks, _connsAmt * 2, _timeout);
#ifdef DEBUG_SERVER_MESSAGES
		std::cout << "Just poll'd, ret number is " << _retCode << std::endl;
#endif

		if (_retCode < 0)
		{
			if (killMe)
			{
				for (int i = 0; i < _connsAmt * 2; i++)
				{
					_purgeOneConnection(i);
				}
				return ;
			}
			throw pollError();
		}
		for (int i = _lstnN; i < _connsAmt; i++)
		{
			if (_perConnArr[i] != NULL)
			{
//				std::cout << _perConnArr[i]->getKeepAlive() << " <- keepalive for " << i << std::endl;
				if ((!(_perConnArr[i]->getKeepAlive()) || (_perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted()))
						&& !(_perConnArr[i]->getStillResponding()) && !(_perConnArr[i]->getSendingFile()) && !(_perConnArr[i]->getWritingFile()))
				{
					_purgeOneConnection(i);
				}
				else
				{
					_debugMsgI(i, "not killed");
					_debugMsgI(time(NULL) - _perConnArr[i]->getTimeStarted(), "<- alive for");
				}
			}
		}
		// go thru all the socks that could have possibly been affected
		for (int i = 0; i < _connsAmt; i++)
		{
			if (_socks[i].fd == -1)
				continue ;
			if (_socks[i].revents == 0)
			{
				_debugMsgI(i, "0 revents");
				continue ;
			}
			else if (((_socks[i].revents & POLLERR) == POLLERR) || ((_socks[i].revents & POLLHUP) == POLLHUP) || ((_socks[i].revents & POLLNVAL) == POLLNVAL))
			{
				if (i < _lstnN)
				{
					_debugMsgI(i, "listening sock got an err/hup/val");
					continue ;
				}
				_debugMsgI(i, "regular sock got an err/hup/val");
				_purgeOneConnection(i);
			}
			else if ((_socks[i].revents & POLLIN) == POLLIN)
			{
				if (i < _lstnN)
				{
					// we have received something on a listening socket
					_connectHereIndex = _checkAvailFdI();
					if (_connectHereIndex != -1)
					{
						_newConnect = accept(_listenSocks[i], NULL, NULL);
						while (_newConnect > 0)
						{
							_acceptNewConnect(i);
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
					// we have something on a normal socket
					std::memset(buf, 0, sizeof (buf));
					_retCode = recv(_socks[i].fd, buf, _rbufSize, 0);
					if (_retCode < 0)
					{
						_debugMsgI(i, "negative retcode");
						_purgeOneConnection(i);
					}
					else if (_retCode == 0)
					{
						_debugMsgI(i, "_retCode is 0, but poll says POLLIN and no errors(?). Maybe we forgot to clean up?");
						if (_localRecvBuffers[i].size() != 0)
						{
							if (_perConnArr[i]->getNeedsBody())
							{
								if (_perConnArr[i]->getContLen() < _localRecvBuffers[i].size())
								{
//									std::cout << _perConnArr[i]->getContLen() << ", " << _localRecvBuffers[i].size() << std::endl;
									_contentTooBigHandilng(i);
								}
								else if (_perConnArr[i]->getContLen() == _localRecvBuffers[i].size())
								{
									// it seems that we're done reading the body then.
									if (_perConnArr[i]->getIsCgi())
									{
										_debugMsgI(i, "cgi post hit on a retcode 0");
									}
									_postReadingIsDone(i);
								}
								// else -- nothing. just wait.
							}
							else if (_localRecvBuffers[i].find(CRLFCRLF) != std::string::npos ||
									_localRecvBuffers[i].find(LFCRLF) != std::string::npos ||
									_localRecvBuffers[i].find(CRLFLF) != std::string::npos ||
									_localRecvBuffers[i].find(LFLF) != std::string::npos)
							{
								_onHeadLocated(i);
							}
							else
							{
								_debugMsgI(i, "No double-endline, the buf isn't zero, nothing to read.");
								if ((!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted())
										&& (!_perConnArr[i]->getStillResponding()) && (!_perConnArr[i]->getSendingFile()) && (!_perConnArr[i]->getWritingFile()))
								{
									_debugMsgI(i, "closing");
									_purgeOneConnection(i);
								}
							}
						}
						else
						{
							_debugMsgI(i, "Nothing in local buf, the read is 0, so weird that we got polled. closing");
							_purgeOneConnection(i);
						}
					}
					else if (_retCode > 0)
					{
						_perConnArr[i]->setTimeStarted(time(NULL));
						_debugMsgI(i, "reset starting time");
						// maybe: bare CR to SP replace
						// (?)
						// https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-4
						// might be useful for raw data, if we ever do it
						buf[_retCode] = 0;
						_localRecvBuffers[i] += std::string(buf);

						if (_perConnArr[i]->getNeedsBody())
						{
							if (_perConnArr[i]->getContLen() < _localRecvBuffers[i].size())
							{
//								std::cout << _perConnArr[i]->getContLen() << ", " << _localRecvBuffers[i].size() << std::endl;
								_contentTooBigHandilng(i);
							}
							else if (_localRecvBuffers[i].size() == _perConnArr[i]->getContLen())
							{
								if (_perConnArr[i]->getIsCgi())
								{
									_debugMsgI(i, "cgi post hit on a normal retcode");
								}
								_localFWriteBuffers[i] += std::string(buf);
								_postReadingIsDone(i);
							}
							else
							{
								_localFWriteBuffers[i] += std::string(buf);
							}
						}
						else if (_localRecvBuffers[i].find(CRLFCRLF) != std::string::npos ||
								_localRecvBuffers[i].find(LFCRLF) != std::string::npos ||
								_localRecvBuffers[i].find(CRLFLF) != std::string::npos ||
								_localRecvBuffers[i].find(LFLF) != std::string::npos)
						{
							_onHeadLocated(i);
						}
					}
				}
			} /* else if POLLIN */
			else if ((_socks[i].revents & POLLOUT) == POLLOUT)
			{
#ifdef DEBUG_SERVER_MESSAGES
				std::cout << "sending file: " << _perConnArr[i]->getSendingFile() << std::endl;
				std::cout << "is cgi: " << _perConnArr[i]->getIsCgi() << std::endl;
#endif
				// regular sends (not first sends)
				// step 1. are we sending a file rn? no? well, just send the string that's saved in the perconarr[i]
				if (!_perConnArr[i]->getSendingFile())
				{
					if (static_cast<size_t>(_sbufSize) < _perConnArr[i]->getSendStr().size())
					{
						_perConnArr[i]->setTimeStarted(time(NULL));
						_localSendStrings[i] = _perConnArr[i]->getSendStr().substr(0, _sbufSize);
						send(_socks[i].fd, _localSendStrings[i].c_str(), _sbufSize, 0);

						_perConnArr[i]->eraseSendStr(0, _sbufSize);
					}
					else
					{
						_perConnArr[i]->setTimeStarted(time(NULL));
						send(_socks[i].fd, _perConnArr[i]->getSendStr().c_str(), _perConnArr[i]->getSendStr().size(), 0);
						_perConnArr[i]->setStillResponding(false);

						_perConnArr[i]->eraseSendStr(0, _perConnArr[i]->getSendStr().size());
						// maybe, cgi send needs to have its own flag XXX
						if (_perConnArr[i]->getHasFile())
						{
							_debugMsgI(i, "switching to filesending mode");
//							std::cout << "i should send this: " << _socks[i + _connsAmt].fd << std::endl;
							_perConnArr[i]->setSendingFile(true);
						}
						else
						{
							_socks[i].events = POLLIN;
						}
					}
				}
				else if (_perConnArr[i]->getSendingFile() && !_perConnArr[i]->getIsCgi())
				{
					_tempFdI = i + _connsAmt;
					if (((_socks[_tempFdI].revents & POLLERR) == POLLERR)
							|| ((_socks[_tempFdI].revents & POLLHUP) == POLLHUP)
							|| ((_socks[_tempFdI].revents & POLLNVAL) == POLLNVAL))
					{
						_debugMsgI(i, "socket's file got an err/hup/nval");
						_purgeOneConnection(i);
						continue ;
					}
					else if ((_socks[_tempFdI].revents & POLLIN) == POLLIN)
					{
						std::memset(fileToReadBuf, 0, sizeof (fileToReadBuf));
						_retCode = read(_socks[_tempFdI].fd, fileToReadBuf, _rbufSize);
						if (_retCode < 0)
						{
							_debugMsgI(i, "retcode on file is < 0");
							_purgeOneConnection(i);
							continue ;
						}
						else if (_retCode == 0)
						{
							_debugMsgI(i, "retcode on file is 0");
							// we somehow had something from poll but showed up with 0 bytes upon actual reading.
							_perConnArr[i]->setTimeStarted(time(NULL));
							_cleanAfterNormalRead(i);
						}
						else if (_perConnArr[i]->getRemainingFileSize() == _retCode)
						{
							_debugMsgI(i, "perfect retcode on file!");
							// the amount of file left is exactly the amount that was read. ggs
							_perConnArr[i]->setTimeStarted(time(NULL));
							fileToReadBuf[_retCode] = 0;
							_localSendStrings[i] = std::string(fileToReadBuf);
							send(_socks[i].fd, _localSendStrings[i].c_str(), _retCode + 1, 0);
							_cleanAfterNormalRead(i);
						}
						else
						{
							// the "regular". we need to substract the sbuf size from the counter of remaining filesize and send the chunk.
							_perConnArr[i]->setTimeStarted(time(NULL));

							fileToReadBuf[_retCode] = 0;
							_localSendStrings[i] = std::string(fileToReadBuf);
							send(_socks[i].fd, _localSendStrings[i].c_str(), _sbufSize, 0);

							_perConnArr[i]->diminishRemainingFileSize(_sbufSize);
						}
					}
					else
					{
						_debugMsgI(i, "file not ready yet");
						_debugMsgI(_tempFdI, "<- file");
						_debugMsgI(_socks[_tempFdI].fd, "<- file's fd");
						continue ;
					}
				}
				else if (_perConnArr[i]->getIsCgi())
				{
					_tempFdI = i + _connsAmt;
					if (((_socks[_tempFdI].revents & POLLERR) == POLLERR)
							|| ((_socks[_tempFdI].revents & POLLHUP) == POLLHUP)
							|| ((_socks[_tempFdI].revents & POLLNVAL) == POLLNVAL))
					{
						_debugMsgI(i, "socket's cgi got an err/hup/nval");
						_purgeOneConnection(i);
						continue ;
					}
					else if ((_socks[_tempFdI].revents & POLLIN) == POLLIN)
					{
						_debugMsgI(i, "pollin on tempfd sock");
						std::memset(fileToReadBuf, 0, sizeof (fileToReadBuf));
						_retCode = read(_socks[_tempFdI].fd, fileToReadBuf, _rbufSize);
						if (_retCode < 0)
						{
							_debugMsgI(i, "retcode on file is < 0");
							_purgeOneConnection(i);
							continue ;
						}
						else if (_retCode == 0)
						{
							_debugMsgI(i, "retcode ZERO");
							if (!_localSendStrings[i].empty())
							{
								_debugMsgI(i, "entered in the polled but ret = 0 state with some dangling tail from cgi");
								// the remnants
								_perConnArr[i]->setTimeStarted(time(NULL));

								if (_localSendStrings[i].size() <= static_cast<size_t>(_sbufSize))
								{
									send(_socks[i].fd, _localSendStrings[i].c_str(), _localSendStrings[i].size(), 0);
									_localSendStrings[i].clear();
								}
								else
								{
									send(_socks[i].fd, _localSendStrings[i].c_str(), _sbufSize, 0);
									_localSendStrings[i].erase(0, _sbufSize);
								}
							}
							else
							{
								_perConnArr[i]->setTimeStarted(time(NULL));
								_cleanAfterNormalRead(i);
								// TODO close connection?
							}
						}
						else if (_perConnArr[i]->getFirstTimeCgiSend())
						{
							_debugMsgI(i, "cgi's first time send!");
							fileToReadBuf[_retCode] = 0;
							_perConnArr[i]->setFirstTimeCgiSend(false);
							_perConnArr[i]->setTimeStarted(time(NULL));
							_localSendStrings[i] = std::string("HTTP/1.1 ") + _parseCgiStatus(fileToReadBuf) + CRLF;
							_localSendStrings[i] += fileToReadBuf;
							if (_localSendStrings[i].size() <= static_cast<size_t>(_sbufSize))
							{
								send(_socks[i].fd, _localSendStrings[i].c_str(), _localSendStrings[i].size(), 0);
								_localSendStrings[i].clear();
							}
							else
							{
								send(_socks[i].fd, _localSendStrings[i].c_str(), _sbufSize, 0);
								_localSendStrings[i].erase(0, _sbufSize);
							}
						}
						else
						{
							_debugMsgI(i, "cgi's regular send!");
							// the "regular". we need to diminish the local send str by what was sent.
							_perConnArr[i]->setTimeStarted(time(NULL));

							fileToReadBuf[_retCode] = 0;
							_localSendStrings[i] += fileToReadBuf;
							if (_localSendStrings[i].size() <= static_cast<size_t>(_sbufSize))
							{
								send(_socks[i].fd, _localSendStrings[i].c_str(), _localSendStrings[i].size(), 0);
								_localSendStrings[i].clear();
							}
							else
							{
								send(_socks[i].fd, _localSendStrings[i].c_str(), _sbufSize, 0);
								_localSendStrings[i].erase(0, _sbufSize);
							}
						}
					}
					else
					{
						_debugMsgI(i, "revents aren't pollin on the cgi sock......");
						if (!_localSendStrings[i].empty())
						{
							// the remnants
							_perConnArr[i]->setTimeStarted(time(NULL));

							if (_localSendStrings[i].size() <= static_cast<size_t>(_sbufSize))
							{
								send(_socks[i].fd, _localSendStrings[i].c_str(), _localSendStrings[i].size(), 0);
								_localSendStrings[i].clear();
							}
							else
							{
								send(_socks[i].fd, _localSendStrings[i].c_str(), _sbufSize, 0);
								_localSendStrings[i].erase(0, _sbufSize);
							}
						}
						else
						{
#ifdef DEBUG_SERVER_MESSAGES
							std::cout << time(NULL) << std::endl;
							std::cout << _perConnArr[i]->getCgiTimeStarted() << std::endl;
							std::cout << _perConnArr[i]->getCgiTimeout() << std::endl;
							std::cout << _perConnArr[i]->getTimeStarted() << std::endl;
							std::cout << _perConnArr[i]->getKaTimeout() << std::endl;
#endif
							if (waitpid(_perConnArr[i]->getPid(), NULL, WNOHANG) == -1)
							{
								_debugMsgI(i, "child cgi died itself");
								if (_perConnArr[i]->getFirstTimeCgiSend())
								{
									// we didn't send anything, execve probably failed.
									_cleanAfterCatching(i);
									_perConnArr[i]->setIsCgi(false);
									ResponseGenerator	responseObject("500 Internal Server Error", _perConnArr[i]->getServerContext());
									if (responseObject.getHasFile())
									{
										_responseObjectHasAFile(i, &responseObject);
									}
									_firstTimeSender(&responseObject, i, true, true);
								}
								else
								{
									_purgeOneConnection(i);
								}
							}
							else if (time(NULL) - _perConnArr[i]->getCgiTimeStarted() > _perConnArr[i]->getCgiTimeout())
							{
								kill(_perConnArr[i]->getPid(), 9);
								_debugMsgI(i, "child cgi killed manually by timeout");
								// socks events is already pollout
								_cleanAfterCatching(i);
								_perConnArr[i]->setIsCgi(false);
								_perConnArr[i]->setFirstTimeCgiSend(true);
								ResponseGenerator	responseObject("502 Bad Gateway", _perConnArr[i]->getServerContext());
								if (responseObject.getHasFile())
								{
									_responseObjectHasAFile(i, &responseObject);
								}

								_firstTimeSender(&responseObject, i, true, true);
//								_purgeOneConnection(i);
							}
						}
					}
				}
			} /* else if POLLOUT */
			if (i >= _lstnN && _perConnArr[i] != NULL)
			{
#ifdef DEBUG_SERVER_MESSAGES
				std::cout << i << std::endl;
				std::cout << "ka " << _perConnArr[i]->getKeepAlive() << std::endl;
				std::cout << "to " << (_perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted()) << std::endl;
				std::cout << "sr " << _perConnArr[i]->getStillResponding() << std::endl;
				std::cout << "sf " << _perConnArr[i]->getSendingFile() << std::endl;
				std::cout << "wf " << _perConnArr[i]->getWritingFile() << std::endl;
#endif
				if ((!_perConnArr[i]->getKeepAlive() || _perConnArr[i]->getKaTimeout() < time(NULL) - _perConnArr[i]->getTimeStarted())
						&& (!_perConnArr[i]->getStillResponding()) && (!_perConnArr[i]->getSendingFile()) && (!_perConnArr[i]->getWritingFile()))
				{
					_purgeOneConnection(i);
				}
			}
		} /* for to iterate thru _socks upon poll's return */
		// time to iterate thru the files! (or the cgis)
		for (int i = _connsAmt; i < _connsAmt * 2; i++)
		{
			if (_socks[i].fd == -1 || ((_socks[i].events & POLLIN) == POLLIN) || (_perConnArr[i - _connsAmt] != NULL && _perConnArr[i - _connsAmt]->getIsCgi()))
			{
				// pollin is done in the socket part
				continue ;
			}
			if (_socks[i].revents == 0)
			{
				_debugMsgI(i, "revents is 0");
				if ((!_perConnArr[i - _connsAmt]->getKeepAlive() || _perConnArr[i - _connsAmt]->getKaTimeout() < time(NULL) - _perConnArr[i - _connsAmt]->getTimeStarted()))
				{
					_purgeOneConnection(i - _connsAmt);
				}
				continue ;
			}
			else if (((_socks[i].revents & POLLERR) == POLLERR) || ((_socks[i].revents & POLLHUP) == POLLHUP) || ((_socks[i].revents & POLLNVAL) == POLLNVAL))
			{
				_debugMsgI(i, "got an err/hup/val");
				_purgeOneConnection(i - _connsAmt);
				continue ;
			}
			else if ((_socks[i].revents & POLLOUT) == POLLOUT)
			{
				if (static_cast<size_t>(_rbufSize) < _localFWriteBuffers[i - _connsAmt].size())
				{
					write(_socks[i].fd, _localFWriteBuffers[i - _connsAmt].c_str(), _rbufSize);
					_debugMsgI(i, "I just wrote something using a regular write 1.");
					_fWCounts[i - _connsAmt] += _rbufSize;
					_localFWriteBuffers[i - _connsAmt].erase(0, _rbufSize);
					_localRecvBuffers[i - _connsAmt].erase(0, _rbufSize);
					_perConnArr[i - _connsAmt]->setTimeStarted(time(NULL));
				}
				else
				{
					write(_socks[i].fd, _localFWriteBuffers[i - _connsAmt].c_str(), _localFWriteBuffers[i - _connsAmt].size());
					_debugMsgI(i, "I just wrote something using a regular write 2.");
					if (_localFWriteBuffers[i - _connsAmt].size() > 0)
					{
						_perConnArr[i - _connsAmt]->setTimeStarted(time(NULL));
					}
					_fWCounts[i - _connsAmt] += _localFWriteBuffers[i - _connsAmt].size();
					_localFWriteBuffers[i - _connsAmt].clear();
					_localRecvBuffers[i - _connsAmt].erase(0, _rbufSize);
				}
			}
			if (_fWCounts[i - _connsAmt] == _perConnArr[i - _connsAmt]->getContLen())
			{
				_debugMsgI(i, "done posting.");
				close(_socks[i].fd);
				_fWCounts[i - _connsAmt] = 0;
				_socks[i].fd = -1;
				_socks[i].events = 0;
				_socks[i].revents = 0;
				// must send the 201 created now
				_socks[i - _connsAmt].events = POLLOUT;
				_perConnArr[i - _connsAmt]->setWritingFile(false);
				_perConnArr[i - _connsAmt]->setNeedsBody(false);
				_perConnArr[i - _connsAmt]->setStillResponding(true);
			}
			else if (_fWCounts[i - _connsAmt] > _perConnArr[i - _connsAmt]->getContLen() && !_perConnArr[i - _connsAmt]->getIsCgi())
			{
				_debugMsgI(i, "too much data. deleting the file");
				remove(_perConnArr[i]->getRTarget().c_str());
				_purgeOneConnection(i - _connsAmt);
			}
			else if ((!_perConnArr[i - _connsAmt]->getKeepAlive() || _perConnArr[i - _connsAmt]->getKaTimeout() < time(NULL) - _perConnArr[i - _connsAmt]->getTimeStarted()))
			{
				_purgeOneConnection(i - _connsAmt);
			}
		} /* for files in _socks */
#ifdef DEBUG_SERVER_MESSAGES
		std::cout << "                one cycle done!" << std::endl;
#endif
	} /* while (_running || !killMe) */
	for (int i = 0; i < _connsAmt * 2; i++)
	{
		_purgeOneConnection(i);
	}
}

Server::~Server()
{
	delete _grandConfig;
	for (size_t i = 0; i < _listenSocks.size(); i++)
	{
		delete _perConnArr[i];
	}
}
