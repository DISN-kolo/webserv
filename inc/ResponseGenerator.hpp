#ifndef RESPONSE_GENERATOR_HPP
# define RESPONSE_GENERATOR_HPP
# include "webserv.hpp"
# include "RequestHeadParser.hpp"

# include <sstream>
# include <dirent.h>
# include <sys/wait.h>

class ResponseGenerator
{
private:
	ResponseGenerator();
	ResponseGenerator(const ResponseGenerator & obj);
	ResponseGenerator &operator=(const ResponseGenerator & obj);

	void	_defaultErrorPageGenerator(const char * ewhat);
	std::string	_getContent(int code);
	std::string	_getStatusMessage(int status);
	std::string	_getDate(void);
	std::string	_getServerName(void);
	std::string	_getContentType(void);

	std::string	_generateListing(std::string dirpath, std::string apparentTarget, struct config_server_t server);

	char						**_env;
	std::vector<std::string>	_envPaths;
	int							_execCGI(const RequestHeadParser &req, const config_server_t &config);

	std::string	_text;
	off_t		_fSize;
	int			_fd;
	bool		_hasFile;

	struct config_server_t	_server;
public:
	ResponseGenerator(int code);
	ResponseGenerator(const char * ewhat, struct config_server_t server);
	ResponseGenerator(const RequestHeadParser & req, struct config_server_t server, char **env, std::vector<std::string> envPaths);
	~ResponseGenerator();

	std::string	getText(void) const;
	size_t		getSize(void) const;
	off_t		getFSize(void) const;
	int			getFd(void) const;
	bool		getHasFile(void) const;
} ;
#endif
