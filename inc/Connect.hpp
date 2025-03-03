#ifndef CONNECT_HPP
# define CONNECT_HPP

# include "webserv.hpp"
# include "ServerConfig.hpp"

// connect needs to know:
// 1. if it needs to continue (this will be known after a req head parse)
// 2. if it does, for how long (as in N of characters)
// 2.1. maybe even the temp body string???
// 3. keep-alive status
// 4. transfer-encoding: chunked (? ? ?)
// it's a solution to handling multiple aux parameters per established connection
class	Connect
{
private:
	Connect(const Connect & obj);
	Connect &operator=(const Connect & obj);

	bool					_needsBody;
	size_t					_contLen;
	bool					_keepAlive;
	time_t					_timeStarted;
	time_t					_kaTimeout;
	std::string				_sendStr;
	bool					_stillResponding;
	bool					_hasFile;
	bool					_sendingFile;
	bool					_writingFile;
	int						_fd;
	off_t					_remainingFileSize;
	struct config_server_t	_serverContext;
	std::string				_rTarget;
public:
	Connect();
	~Connect();

	bool					getNeedsBody(void) const;
	size_t					getContLen(void) const;
	bool					getKeepAlive(void) const;
	time_t					getTimeStarted(void) const;
	time_t					getKaTimeout(void) const;
	std::string				getSendStr(void) const;
	bool					getStillResponding(void) const;
	bool					getHasFile(void) const;
	bool					getSendingFile(void) const;
	bool					getWritingFile(void) const;
	int						getFd(void) const;
	off_t					getRemainingFileSize(void) const;
	struct config_server_t	getServerContext(void) const;
	std::string				getRTarget(void) const;

	void	setNeedsBody(bool v);
	void	setContLen(size_t v);
	void	setKeepAlive(bool v);
	void	setTimeStarted(time_t v);
	void	setKaTimeout(time_t v);
	void	setSendStr(std::string v);
	void	setStillResponding(bool v);
	void	setHasFile(bool v);
	void	setSendingFile(bool v);
	void	setWritingFile(bool v);
	void	setFd(int v);
	void	setRemainingFileSize(off_t v);
	void	setServerContext(struct config_server_t v);
	void	setRTarget(std::string v);
	void	setPortInUse(int port);

	void	eraseSendStr(size_t pos, size_t len);
	void	diminishRemainingFileSize(int amt);
} ;

#endif
