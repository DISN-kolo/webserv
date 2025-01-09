#include "../inc/webserv.hpp"

// dummy function.
// real function is probably like:
// 200? get something that was required.
// anything else? get a standard eror page OR a page specified by the config.
// => needs access to config
std::string	getContent(int code)
{
	if (code == 200)
		return ("<head>Hello, my wonderful friends!</head><body>This is an html body :3</body>");
	else if (code == 404)
		return ("<head align=\"center\">404 Not Found</head>");
	else
		return ("A secret third option");
}

// TODO proably a good idea to make a map of statuses
std::string	getStatusMessage(int status)
{
	switch (status)
	{
		case 200:
			return ("OK");
		case 404:
			return ("Not Found");
		default:
			return ("I'm a teapot");
	}
}

// TODO days can probably be a class variable. and months. also const lol
std::string	getDate(void)
{
	std::stringstream	ss;
	time_t	curTime = time(NULL);
	std::string	days[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	std::string	months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "July", "Aug", "Sep", "Oct", "Nov", "Dec"};
	std::string	ret;
	struct tm	dateTime = *gmtime(&curTime);
	ss << days[dateTime.tm_wday] << ", ";
	if (dateTime.tm_mday < 10)
		ss << "0";
	ss << dateTime.tm_mday << " ";
	ss << months[dateTime.tm_mon] << " ";
	ss << (dateTime.tm_year + 1900) << " ";
	if (dateTime.tm_hour < 10)
		ss << "0";
	ss << dateTime.tm_hour << ":";
	if (dateTime.tm_min < 10)
		ss << "0";
	ss << dateTime.tm_min << ":";
	if (dateTime.tm_sec < 10)
		ss << "0";
	ss << dateTime.tm_sec;
	ret = ss.str();
	return (ret);
}

std::string	getServerName(void)
{
	return ("localhost");
}

std::string	getContentType(void)
{
	return ("text/html");
}

// TODO in prod, probably make this a class' function (all the get methods will in turn be internal to that class or some other aux class probably)
std::string	responseGenerator(int status)
{
	std::stringstream	ss;
	std::string	resp;
	std::string	cont = getContent(200);
	ss << "HTTP/1.1 " << status << " " << getStatusMessage(status) << CRLF;
	ss << "Date: " << getDate() << CRLF;
	ss << "Server: " << getServerName() << CRLF;
	ss << "Content-Type: " << getContentType() << CRLF;
	ss << "Content-Length: " << cont.size() << CRLF;
	ss << CRLF;
	ss << cont;
	ss << CRLF;
	resp = ss.str();
	return (resp);
}

#include <map>
#include <vector>
int	main(int argc, char **argv)
{
	std::map<short, std::string> pe;
	for (short enumval = 0; enumval < POLLWRBAND*2; enumval++)
		pe[enumval] = "some combo or something else";
	pe[0] = "nope";
	pe[POLLIN] = "POLLIN";
	pe[POLLIN | POLLERR] = "POLLIN | POLLERR";
	pe[POLLIN | POLLHUP] = "POLLIN | POLLHUP";
    pe[POLLPRI] = "POLLPRI";
    pe[POLLOUT] = "POLLOUT";
    pe[POLLRDHUP] = "POLLRDHUP";
    pe[POLLERR] = "POLLERR";
    pe[POLLHUP] = "POLLHUP";
    pe[POLLNVAL] = "POLLNVAL";
    pe[POLLRDNORM] = "POLLRDNORM";
    pe[POLLRDBAND] = "POLLRDBAND";
    pe[POLLWRNORM] = "POLLWRNORM";
    pe[POLLWRBAND] = "POLLWRBAND";

	// port setup TODO this has to be done using config
	int	port1;
	int	port2;
	std::vector<int>	ports;
	if (argc == 1)
	{
		ports.push_back(9000);
		port1 = 9000;
		port2 = 9000;
	}
	else if (argc == 2)
	{
		port1 = std::atoi(argv[1]);
		if ((port1 < 1024) || (port1 > 65535))
			throw badPortError();
		ports.push_back(port1);
		port2 = port1;
	}
	else if (argc >= 3)
	{
		port1 = std::atoi(argv[1]);
		if ((port1 < 1024) || (port1 > 65535))
			throw badPortError();
		port2 = std::atoi(argv[2]);
		if ((port2 < 1024) || (port2 > 65535))
			throw badPortError();
		if (port1 > port2)
		{
			int	c = port1;
			port1 = port2;
			port2 = c;
		}
		else if (port1 == port2)
		{
			ports.push_back(port1);
		}
		if (port1 != port2)
		{
			for (int port = port1; port < port2; port++)
			{
				ports.push_back(port);
			}
			ports.push_back(port2);
		}
	}
	if (ports.size() > BLOG_SIZE/2)
		throw tooManyPorts();


	int	listenSock;
	std::vector<int> listenSocks;
	int	reuseAddressValue;
	sockaddr_in	addresses[ports.size()];
	for (unsigned long i = 0; i < ports.size(); i++)
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
		addresses[i].sin_port = htons(ports[i]);
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
	return (0);
}
