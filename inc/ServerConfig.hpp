#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include "webserv.hpp"
# include "exceptions.hpp"
# include <fstream>
# include <sstream>

struct routes {
	std::vector<std::string>	accMethods;
	std::vector<std::string>	cgiEx;
	std::vector<std::string>	cgiPath;
	std::vector<std::string>	cgiFolder;
	std::string					name;
	std::string					alias;
	std::string					redirection;
	std::string					root;
	std::string					index;
	bool						autoIndex;
	bool						cgiEnable;
	int							maxBodySize;
};

struct config {
	std::vector<std::pair<std::string, std::string> >	customErrors;
	std::vector<std::string>	accMethods;
	std::vector<std::string>	cgiEx;
	std::vector<std::string>	cgiPath;
	std::vector<std::string>	cgiFolder;
	std::vector<int>			ports;
	std::string					host;
	std::string					name;
	std::string					root;
	bool						cgiEnable;
	bool						autoIndex;
	int							maxBodySize;
	std::vector<struct routes>	routes;
};

class ServerConfig
{
	private:
		std::vector<struct config>	_config;
		void						_parseConfig(const std::string &file);
		void						_parseLine(const std::string &line, int &brackets);
		std::string					_getLine(const std::string &line);
		std::string					_getLineKey(const std::string &line);
		std::vector<std::string>	_getLineValues(const std::string &line);
		std::string					_getLineValue(const std::string &values);
		void						_validateConfigValues(const std::string &key, const std::vector<std::string> values, int &brackets);

		bool						_validateChildValue(const std::vector<std::string> values);
		void						_validateAutoindex(std::vector<std::string> values, int brackets);
		void						_validateMaxBody(std::vector<std::string> values, int brackets);
		void						_validateHost(std::vector<std::string> values, int brackets);
		void						_validateServerName(std::vector<std::string> values, int brackets);
		void						_validateRoot(std::vector<std::string> values, int brackets);
		void						_validateIndex(std::vector<std::string> values, int brackets);
		void						_validateName(std::vector<std::string> values, int brackets);
		void						_validateAlias(std::vector<std::string> values, int brackets);
		void						_validatePort(std::vector<std::string> values, int brackets);
		void						_validateMethods(std::vector<std::string> values, int brackets);
		void						_validateCgiPath(std::vector<std::string> values, int brackets);
		void						_validateCgiExt(std::vector<std::string> values, int brackets);
		void						_validateErrorPage(std::vector<std::string> values, int brackets);

		int							_stoi(const std::string &str);
	public:
		ServerConfig();
		ServerConfig(const std::string &file);
		ServerConfig(const ServerConfig & obj);
		ServerConfig &operator=(const ServerConfig & obj);
		~ServerConfig();
		std::vector<int>getPorts();
} ;

#endif
