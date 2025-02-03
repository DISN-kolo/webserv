#ifndef CONNECT_HPP
# define CONNECT_HPP

# include "webserv.hpp"

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

	bool		_needsBody;
	size_t		_contLen;
	bool		_keepAlive;
	time_t		_timeStarted;
	time_t		_kaTimeout;
	std::string	_sendStr;
	bool		_stillResponding;
	bool		_hasAFile;
	bool		_sendingFile;
public:
	Connect();
	~Connect();

	bool		getNeedsBody(void) const;
	size_t		getContLen(void) const;
	bool		getKeepAlive(void) const;
	time_t		getTimeStarted(void) const;
	time_t		getKaTimeout(void) const;
	std::string	getSendStr(void) const;
	bool		getStillResponding(void) const;
	bool		getHasAFile(void) const;
	bool		getSendingFile(void) const;

	void		setNeedsBody(bool v);
	void		setContLen(size_t v);
	void		setKeepAlive(bool v);
	void		setTimeStarted(time_t v);
	void		setKaTimeout(time_t v);
	void		setSendStr(std::string v);
	void		setStillResponding(bool v);
	void		setHasAFile(bool v);
	void		setSendingFile(bool v);

	void		eraseSendStr(size_t pos, size_t len);
} ;

#endif
