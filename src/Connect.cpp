#include "../inc/Connect.hpp"

Connect::Conenct()
	:	_needsBody(false), _contLen(0), _keepAlive(true)
{
}

Connect &Connect::operator=(const Connect & obj)
{
	_needsBody = obj.getNeedsBody();
	_contLen = obj.getContLen();
	_keepAlive = obj.getKeepAlive();
	return (*this);
}

Connect::Conenct(const Connect & obj)
{
	*this = obj;
}

Connect::~Conenct()
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
