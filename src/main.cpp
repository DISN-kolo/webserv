#include "../inc/Server.hpp"

int	main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
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

	return (0);
}
