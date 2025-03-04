#include "../inc/Server.hpp"

int	main(int argc, char **argv, char **env)
{
	try {
		Server	server(argc, argv, env);

		// evil shit
		signal(SIGPIPE, SIG_IGN); 

		server.run();
	} catch (std::exception &err)
	{
		std::cout << err.what() << std::endl;
	}


	return (0);
}
