#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include "webserv.hpp"
# include "exceptions.hpp"
# include <fstream>
# include <sstream>

struct config_location_t {
	std::vector<std::string>	accMethods;
	std::string					redir;
	std::string					name;
	std::string					root;
	std::string					index;
	bool						autoIndex;

	bool						autoIndexF;
};

struct config_server_t {
	std::vector<int>			ports;
	std::string					host;
	std::string					name;
	std::vector<std::pair<std::string, std::string> >	customErrors;
	size_t						maxBodySize;
	std::string					index;
	std::string					root;

	std::vector<struct config_location_t>	locations;
};

class ServerConfig
{
	private:
		std::vector<struct config_server_t>	_config;
		void						_parseHandler(const std::string &file);
		void						_parseConfig(const std::string &file);
		void						_parseLine(const std::string &line, int &brackets);
		std::string					_getLine(const std::string &line);
		std::string					_getLineKey(const std::string &line);
		std::vector<std::string>	_getLineValues(const std::string &line);
		std::string					_getLineValue(const std::string &values);
		void						_validateConfigValues(const std::string &key, const std::vector<std::string> values, int &brackets);

		bool						_validateChildValue(const std::vector<std::string> values);
		void						_validateFileExt(const std::string &file);
		void						_validateBracketClose(const std::string line, int &brackets);
		void						_validateAutoindex(std::vector<std::string> values, int brackets);
		void						_validateMaxBody(std::vector<std::string> values, int brackets);
		void						_validateHost(std::vector<std::string> values, int brackets);
		void						_validateServerName(std::vector<std::string> values, int brackets);
		void						_validateRoot(std::vector<std::string> values, int brackets);
		void						_validateIndex(std::vector<std::string> values, int brackets);
		void						_validateName(std::vector<std::string> values, int brackets);
		void						_validatePort(std::vector<std::string> values, int brackets);
		void						_validateMethods(std::vector<std::string> values, int brackets);
		void						_validateErrorPage(std::vector<std::string> values, int brackets);
		void						_validateReturn(std::vector<std::string> values, int brackets);

		void						_validateConfigRequirements();

		int							_stoi(const std::string &str);
	public:
		ServerConfig();
		ServerConfig(const std::string &file);
		ServerConfig(const ServerConfig & obj);
		ServerConfig &operator=(const ServerConfig & obj);
		~ServerConfig();

		std::vector<struct config_server_t> getConfig();
} ;

#endif
