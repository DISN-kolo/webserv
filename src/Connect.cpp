#include "../inc/Connect.hpp"

Connect::Conenct()
	:	_localRecvBuffer("")
{
}

Connect::Conenct(const Connect & obj)
{
	(void)obj;
}

Connect &Connect::operator=(const Connect & obj)
{
	(void)obj;
	return (*this);
}

Connect::~Conenct()
{
}

std::string	Connect::getLRB(void) const
{
	return (_localRecvBuffer);
}

void		Connect::setLRB(std::string newBuffer)
{
	_localRecvBuffer = newBuffer;
}

void		Connect::clearLRB(void)
{
	_localRecvBuffer.clear();
}
