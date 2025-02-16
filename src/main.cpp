#include "../inc/Server.hpp"

int	main(int argc, char **argv)
{
	Server	server(argc, argv);

	// evil shit
	signal(SIGPIPE, SIG_IGN); 

	try {
		//server.run();
	} catch (std::exception &err)
	{
		std::cout << err.what() << std::endl;
	}
	return (0);
}
