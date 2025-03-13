#include "../inc/Connect.hpp"

Connect::Connect()
	:	_needsBody(false), _contLen(0), _keepAlive(true), _timeStarted(time(NULL)), _cgiTimeStarted(time(NULL)), _kaTimeout(KA_TIME), _cgiTimeout(CGI_TIME), _sendStr(""), _stillResponding(false), _hasFile(false), _sendingFile(false), _writingFile(false), _fd(-1), _remainingFileSize(0), _rTarget(""), _isCgi(false), _firstTimeCgiSend(true), _pid(-2)
{
}

Connect &Connect::operator=(const Connect & obj)
{
	_needsBody = obj.getNeedsBody();
	_contLen = obj.getContLen();
	_keepAlive = obj.getKeepAlive();
	_timeStarted = obj.getTimeStarted();
	_cgiTimeStarted = obj.getCgiTimeStarted();
	_kaTimeout = obj.getKaTimeout();
	_cgiTimeout = obj.getCgiTimeout();
	_sendStr = obj.getSendStr();
	_stillResponding = obj.getStillResponding();
	_hasFile = obj.getHasFile();
	_sendingFile = obj.getSendingFile();
	_writingFile = obj.getWritingFile();
	_fd = obj.getFd();
	_remainingFileSize = obj.getRemainingFileSize();
	_serverContext = obj.getServerContext();
	_rTarget = obj.getRTarget();
	_isCgi = obj.getIsCgi();
	_firstTimeCgiSend = obj.getFirstTimeCgiSend();
	_pid = obj.getPid();
	return (*this);
}

Connect::Connect(const Connect & obj)
{
	*this = obj;
}

Connect::~Connect()
{
}

bool	Connect::getNeedsBody(void) const
{
	return (_needsBody);
}

size_t	Connect::getContLen(void) const
{
	return (_contLen);
}

bool	Connect::getKeepAlive(void) const
{
	return (_keepAlive);
}

time_t	Connect::getTimeStarted(void) const
{
	return (_timeStarted);
}

time_t	Connect::getCgiTimeStarted(void) const
{
	return (_cgiTimeStarted);
}

time_t	Connect::getKaTimeout(void) const
{
	return (_kaTimeout);
}

time_t	Connect::getCgiTimeout(void) const
{
	return (_cgiTimeout);
}

std::string	Connect::getSendStr(void) const
{
	return (_sendStr);
}

bool	Connect::getStillResponding(void) const
{
	return (_stillResponding);
}

bool	Connect::getHasFile(void) const
{
	return (_hasFile);
}

bool	Connect::getSendingFile(void) const
{
	return (_sendingFile);
}

bool	Connect::getWritingFile(void) const
{
	return (_writingFile);
}

int		Connect::getFd(void) const
{
	return (_fd);
}

off_t	Connect::getRemainingFileSize(void) const
{
	return (_remainingFileSize);
}

struct config_server_t	Connect::getServerContext(void) const
{
	return (_serverContext);
}

std::string	Connect::getRTarget(void) const
{
	return (_rTarget);
}

bool	Connect::getIsCgi(void) const
{
	return (_isCgi);
}

bool	Connect::getFirstTimeCgiSend(void) const
{
	return (_firstTimeCgiSend);
}

int		Connect::getPid(void) const
{
	return (_pid);
}

// v for value lol
void	Connect::setNeedsBody(bool v)
{
	_needsBody = v;
}

void	Connect::setContLen(size_t v)
{
	_contLen = v;
}

void	Connect::setKeepAlive(bool v)
{
	_keepAlive = v;
}

void	Connect::setTimeStarted(time_t v)
{
	_timeStarted = v;
}

void	Connect::setCgiTimeStarted(time_t v)
{
	_cgiTimeStarted = v;
}

void	Connect::setKaTimeout(time_t v)
{
	_kaTimeout = v;
}

void	Connect::setCgiTimeout(time_t v)
{
	_cgiTimeout = v;
}

void	Connect::setSendStr(std::string v)
{
	_sendStr = v;
}

void	Connect::setStillResponding(bool v)
{
	_stillResponding = v;
}

void	Connect::setHasFile(bool v)
{
	_hasFile = v;
}

void	Connect::setSendingFile(bool v)
{
	_sendingFile = v;
}

void	Connect::setWritingFile(bool v)
{
	_writingFile = v;
}

void	Connect::setFd(int v)
{
	_fd = v;
}

void	Connect::setRemainingFileSize(off_t v)
{
	_remainingFileSize = v;
}

void	Connect::setServerContext(struct config_server_t v)
{
	_serverContext = v;
}

void	Connect::setRTarget(std::string v)
{
	_rTarget = v;
}

void	Connect::setIsCgi(bool v)
{
	_isCgi = v;
}

void	Connect::setFirstTimeCgiSend(bool v)
{
	_firstTimeCgiSend = v;
}

void	Connect::setPid(int v)
{
	_pid = v;
}

void	Connect::setPortInUse(int port)
{
	_serverContext.ports.clear();
	_serverContext.ports.push_back(port);
}

void	Connect::eraseSendStr(size_t pos, size_t len)
{
	_sendStr.erase(pos, len);
}

void	Connect::diminishRemainingFileSize(int amt)
{
	if (amt > _remainingFileSize)
	{
		_remainingFileSize = 0;
	}
	else
	{
		_remainingFileSize -= amt;
	}
}
