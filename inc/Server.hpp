#ifndef SERVER_HPP
# define SERVER_HPP

# include "ServerConfig.hpp"
# include "ResponseGenerator.hpp"
# include "ServerConfig.hpp"
# include "RequestHeadParser.hpp"
# include "webserv.hpp"
# include "Connect.hpp"

# include <ctime>
# include <unistd.h>
# include <poll.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <arpa/inet.h>
# include <cstdlib>
# include <cstring>
# include <csignal>

void	_gracefulExit(int sig);

class Server
{
private:
	Server();
	Server(const Server & obj);
	Server &operator=(const Server & obj);
	unsigned long	_strIpToUL(std::string ip) const;

	void		_serverRunSetupInit(void);
	int			_checkAvailFdI(void) const;
	void		_acceptNewConnect(int i);
	std::string	_parseCgiStatus(char * fbuf);
	void		_firstTimeSender(ResponseGenerator *rO, int i, bool clearLRB, bool purgeC);
	void		_onHeadLocated(int i);
	void		_cgiPostReadingIsDone(int i);
	void		_eraseDoubleNlInLocalRecvBuffer(int i);
	void		_purgeOneConnection(int i);
	void		_responseObjectHasAFile(int i, ResponseGenerator *responseObject);
	void		_cleanAfterCatching(int i);
	void		_cleanAfterNormalRead(int i);
	void		_contentTooBigHandilng(int i);

	void	_debugMsgI(int i, std::string msg);
	void	_debugMsgTimeI(int i, time_t curTime);

	ServerConfig	*_grandConfig;
	int		_rbufSize;
	int		_sbufSize;
	int		_blogSize;
	int		_connsAmt;

	std::vector<std::string>	_env;
	void						_parseEnv(char **env);

	// internal variables used for the run function and its subfunctions
	struct pollfd				_socks[CONNS_AMT * 2];
	int							_listenSock;
	std::vector<int>			_listenSocks;
	std::vector<std::string>	_localRecvBuffers;
	std::vector<std::string>	_localFWriteBuffers;
	std::vector<size_t>			_fWCounts;
	std::vector<std::string>	_localSendStrings;
	std::vector<Connect * >		_perConnArr;
	int							_reuseAddressValue;
	int 						_timeout;
	int							_lstnN;
	bool						_running;
	int							_connectHereIndex;
	int							_newConnect;
	int							_retCode;
	int							_tempFdI;

	// more specific http parsing helper vars
	size_t						_nlnl;
	std::vector<std::string>	_nls;
public:
	Server(int argc, char **argv, char **env);
	void	run(void);
	~Server();
} ;

#endif
