#include "../inc/ServerConfig.hpp"

ServerConfig::ServerConfig()
{}

ServerConfig::ServerConfig(const std::string &file)
{
	(void) file;
}

ServerConfig::ServerConfig(const ServerConfig & obj)
{
	*this = obj;
}

ServerConfig &ServerConfig::operator=(const ServerConfig & obj)
{
	_config = obj._config;
	return (*this);
}

ServerConfig::~ServerConfig()
{
}
