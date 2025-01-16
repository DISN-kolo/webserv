#ifndef SERVER_HPP
# define SERVER_HPP

# include "ServerConfig.hpp"
# include "ResponseGenerator.hpp"
# include "RequestHeadParser.hpp"
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

	void	_onHeadLocated(int i, int *fdp);

	ServerConfig		*_config;

	// internal variables used for the run function
	int								_listenSock;
	std::vector<int>				_listenSocks;
	std::vector<std::string>		_localRecvBuffers;
	std::vector<Connect *>			_perConnArr;
	int								_reuseAddressValue;
	int 							_timeout;
	int								_socksN;
	int								_lstnN;
	int								_curSize;
	bool							_running;
	int								_newConnect;
	bool							_compressTheArr;
	int								_retCode;
public:
	Server();
	void	run(void);
	~Server();
} ;

#endif
