#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

class ServerConfig
{
private:
	ServerConfig(const ServerConfig & obj);
	ServerConfig &operator=(const ServerConfig & obj);

	std::vector<int>	_ports;
	std::string			_hostname;
public:
	ServerConfig();
	~ServerConfig();

	const std::vector<int>	getPorts(void) const;
} ;

#endif
