#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include "webserv.hpp"
# include "exceptions.hpp"
#include <fstream>

struct routes {
	std::vector<std::string>	accMethods;
	std::vector<std::string>	cgiEx;
	std::string					redirection;
	std::string					root;
	std::string					altDirectory;
	bool						listen;
};

struct config {
	int			port;
	int			maxBodySize;
	std::string	host;
	std::string	name;
	std::string	customErrors;
	std::vector<struct routes>	routes;
};

class ServerConfig
{
private:
	std::vector<struct config>	_config;
	void						_parseConfig(const std::string &file);
	bool						_getConfigLine(const std::string &line);
	std::vector<std::string>	_getConfigValues(const std::string &str);
	std::string					_getConfigValue(const std::string &str);


public:
	ServerConfig();
	ServerConfig(const std::string &file);
	ServerConfig(const ServerConfig & obj);
	ServerConfig &operator=(const ServerConfig & obj);
	~ServerConfig();
	std::vector<int>getPorts();
} ;

#endif
