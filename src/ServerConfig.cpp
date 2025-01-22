#include "../inc/ServerConfig.hpp"

ServerConfig::ServerConfig()
{
	_parseConfig("default.conf");
}

ServerConfig::ServerConfig(const std::string &file)
{
	_parseConfig(file);
}

ServerConfig::ServerConfig(const ServerConfig & obj)
{
	*this = obj;
}

ServerConfig &ServerConfig::operator=(const ServerConfig & obj)
{
	(void) obj;
	return (*this);
}

ServerConfig::~ServerConfig()
{
}

std::vector<int>ServerConfig::getPorts()
{
	return std::vector<int>();
}


void	ServerConfig::_parseConfig(const std::string &file)
{
	std::ifstream	configFile(("config/" + file).c_str());
	std::string		line;
	
	if (!configFile.is_open())
		throw configFileException();
	while (std::getline(configFile, line))
	{
		try {
			_getConfigLine(line);
		} catch (std::exception &err)
		{
			std::cout << "ERROR LINE!" << std::endl;
		}
	}
	configFile.close();
}

bool	ServerConfig::_getConfigLine(const std::string &str)
{
	std::vector<std::string>	values;
	std::string					key;
	int							i = 0, j = 0;

	while (str[i] && std::isspace(str[i]))
		i++;
	if (str[i] == '}')
		return true; // End server / location;
	if (!str[i] || str[i] == '#')
		return false;
	while (str[i + j] && str[i + j] != ':')
		j++;
	if (j == 0)
		throw configFileException();
	key = str.substr(i, j);
	std::cout << "KEY: " << key << " Values: ";
	values = _getConfigValues(str.substr(i + j + 1));
	return false;
}

std::vector<std::string> ServerConfig::_getConfigValues(const std::string &str)
{
	std::vector<std::string>	values;
	std::string					value;
	int	i = 0, j;

	while (str[i])
	{
		j = 0;
		while (str[i + j] && str[i + j] != ',' && str[i + j] != ';')
			j++;
		value = _getConfigValue(str.substr(i, j));
		if (!value.size())
			throw configFileException(); // , without value
		values.push_back(value);
		std::cout << value << "|";
		if (str[i + j] == ';' || (values.size() == 1 && values[values.size() - 1] == "{"))
			break;
		if (!str[i + j])
			throw configFileException(); // end of line without ;
		i += j + 1;
	}
	std::cout << std::endl;
	return values;
}

std::string ServerConfig::_getConfigValue(const std::string &str)
{
	int	i = 0, j = 0;

	while (str[i] && std::isspace(str[i]))
		i++;
	while (str[i + j])
	{
		if (str[i + j] == '{')
			return ("{");
		j++;
	}
	return (str.substr(i, j));
}
