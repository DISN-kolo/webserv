#include "../inc/Server.hpp"

int	main(int argc, char **argv)
{
	ServerConfig	*config = NULL;
  
  // evil shit
	signal(SIGPIPE, SIG_IGN); 
	try {

		if (argc == 1)
			*config = ServerConfig();
		else if (argc == 2)
			*config = ServerConfig(argv[1]);
	} catch (std::exception &err)
	{
		std::cout << err.what() << std::endl;
	}
	return (0);
}
