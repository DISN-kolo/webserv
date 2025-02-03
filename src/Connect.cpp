#include "../inc/Connect.hpp"

Connect::Connect()
	:	_needsBody(false), _contLen(0), _keepAlive(true), _timeStarted(time(NULL)), _kaTimeout(5), _sendStr(""), _stillResponding(false), _hasAFile(false), _sendingFile(false)
{
}

Connect &Connect::operator=(const Connect & obj)
{
	_needsBody = obj.getNeedsBody();
	_contLen = obj.getContLen();
	_keepAlive = obj.getKeepAlive();
	_timeStarted = obj.getTimeStarted();
	_kaTimeout = obj.getKaTimeout();
	_sendStr = obj.getSendStr();
	_stillResponding = obj.getStillResponding();
	_hasAFile = obj.getHasAFile();
	_sendingFile = obj.getSendingFile();
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

time_t	Connect::getKaTimeout(void) const
{
	return (_kaTimeout);
}

std::string	Connect::getSendStr(void) const
{
	return (_sendStr);
}

bool	Connect::getStillResponding(void) const
{
	return (_stillResponding);
}

bool	Connect::getHasAFile(void) const
{
	return (_hasAFile);
}

bool	Connect::getSendingFile(void) const
{
	return (_sendingFile);
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

void	Connect::setKaTimeout(time_t v)
{
	_kaTimeout = v;
}

void	Connect::setSendStr(std::string v)
{
	_sendStr = v;
}

void	Connect::setStillResponding(bool v)
{
	_stillResponding = v;
}

void	Connect::setHasAFile(bool v)
{
	_hasAFile = v;
}

void	Connect::setSendingFile(bool v)
{
	_sendingFile = v;
}

void	Connect::eraseSendStr(size_t pos, size_t len)
{
	_sendStr.erase(pos, len);
}
