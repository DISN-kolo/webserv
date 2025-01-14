#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include "webserv.hpp"

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
	ServerConfig(const ServerConfig & obj);
	ServerConfig &operator=(const ServerConfig & obj);
	std::vector<struct config> _config;

public:
	ServerConfig();
	ServerConfig(const std::string &file);
	~ServerConfig();
} ;

#endif
