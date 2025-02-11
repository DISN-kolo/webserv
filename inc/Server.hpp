#ifndef SERVER_HPP
# define SERVER_HPP

# include "ServerConfig.hpp"
# include "ResponseGenerator.hpp"
# include "RequestHeadParser.hpp"
# include "webserv.hpp"
# include "Connect.hpp"

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
/*
	std::cout << std::setw(4) << i << " > " << std::flush;
	std::cout << msg << std::endl;
	*/

class Server
{
private:
	Server(const Server & obj);
	Server &operator=(const Server & obj);

	void	_firstTimeSender(ResponseGenerator *rO, pollfd *socks, int i, bool clearLRB, bool purgeC);
	void	_onHeadLocated(int i, pollfd *sock);
	void	_eraseDoubleNlInLocalRecvBuffer(int i);
	void	_purgeOneConnection(int i, pollfd* socks);

	void	_debugMsgI(int i, std::string msg);
	void	_debugMsgTimeI(int i, time_t curTime);

	ServerConfig		*_config;
	// temporary until config is availabe;
	int	_rbufSize;
	int	_sbufSize;
	int	_blogSize;
	int	_connsAmt;


	// internal variables used for the run function and its subfunctions
	int							_listenSock;
	std::vector<int>			_listenSocks;
	std::vector<std::string>	_localRecvBuffers;
	std::string					_localSendString;
	std::vector<Connect * >		_perConnArr;
	int							_reuseAddressValue;
	int 						_timeout;
	int							_socksN;
	int							_filesN;
	int							_lstnN;
	int							_curSize;
	bool						_running;
	int							_newConnect;
	bool						_compressTheArr;
	int							_retCode;
	int							_tempFdI;

	// more specific http parsing helper vars
	size_t						_nlnl;
	std::vector<std::string>	_nls;
public:
	Server();
	void	run(void);
	~Server();
} ;

#endif
