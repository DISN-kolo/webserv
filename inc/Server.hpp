#ifndef SERVER_HPP
# define SERVER_HPP

class Server
{
private:
	Server(const Server & obj);
	Server &operator=(const Server & obj);

	ServerConfig	*_config;
public:
	Server();
	void	run(void) const;
	~Server();
} ;

#endif
