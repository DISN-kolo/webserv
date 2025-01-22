#include "../inc/Server.hpp"

int	main(int argc, char **argv)
{
	ServerConfig	*config = NULL;
	
	try {

		if (argc == 1)
			*config = ServerConfig();
		else if (argc == 2)
			*config = ServerConfig(argv[1]);
	} catch (std::exception &err)
	{
		std::cout << err.what() << std::endl;
	}

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
	*/
	return (0);
}
