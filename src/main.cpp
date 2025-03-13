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
		std::cerr << err.what() << "\n" << std::endl;
	}


	return (0);
}
