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
// this function should be in the response generator.
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
// this function should be in the response generator.
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

// this function should be in the response generator.
// needs to access the config file for the servname.
std::string	getServerName(void)
{
	return ("localhost");
}

// this function should be in the response generator.
// it probably needs to look ath the request and behave appropriately
std::string	getContentType(void)
{
	return ("text/html");
}

// duh,
// this function should be in the response generator.
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
	/*
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
	*/
	// all of the below also can probably be done in its own class. like, the principle loop could be "Server" or something.
	// port setup TODO this has to be done using config
	try
	{
//		Server	server(argv[1]);
		Server	server;
		server.run();
	}
	catch (std::exception & e)
	{
		std::cerr << e.what() << std::endl;
	}



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

	//	XXX HERE WERE THE RUN() THINGS XXX

	return (0);
}
