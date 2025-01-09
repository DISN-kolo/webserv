#include "../inc/ServerConfig.hpp"

ServerConfig::ServerConfig()
	:	_hostname("localhost")
{
	_ports.push_back(9000);
}

ServerConfig::ServerConfig(const ServerConfig & obj)
{
	(void)obj;
}

ServerConfig &ServerConfig::operator=(const ServerConfig & obj)
{
	(void)obj;
	return (*this);
}

const std::vector<int>	ServerConfig::getPorts(void) const
{
	return (_ports);
}

ServerConfig::~ServerConfig()
{
}
