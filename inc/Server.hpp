#ifndef SERVER_HPP
# define SERVER_HPP

# include "ServerConfig.hpp"
# include "ResponseGenerator.hpp"
# include "ServerConfig.hpp"
# include "webserv.hpp"

# include <ctime>
# include <unistd.h>
# include <poll.h>
# include <sys/types.h>
# include <sys/stat.h>
//# include <netinet/in.h>
# include <arpa/inet.h>
# include <cstdlib>
# include <cstring>
# include <csignal>

class Server
{
private:
	Server(const Server & obj);
	Server &operator=(const Server & obj);

	ServerConfig		*_config;

	// internal variables used for the run function
	int					_listenSock;
	std::vector<int>	_listenSocks;
	int					_reuseAddressValue;
	int 				_timeout;
	int					_socksN;
	int					_lstnN;
	int					_curSize;
	bool				_running;
	int					_newConnect;
	bool				_compressTheArr;
	int					_retCode;
public:
	Server();
	void	run(void);
	~Server();
} ;

#endif
