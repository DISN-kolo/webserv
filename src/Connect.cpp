#include "../inc/Connect.hpp"

Connect::Connect()
	:	_needsBody(false), _contLen(0), _keepAlive(true), _timeStarted(time(NULL)), _kaTimeout(5)
{
}

Connect &Connect::operator=(const Connect & obj)
{
	_needsBody = obj.getNeedsBody();
	_contLen = obj.getContLen();
	_keepAlive = obj.getKeepAlive();
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
