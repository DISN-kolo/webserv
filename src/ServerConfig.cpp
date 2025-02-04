#include "../inc/ServerConfig.hpp"

ServerConfig::ServerConfig()
{
	_parseConfig("config/default.conf");
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
	std::ifstream	configFile((file).c_str());
	std::string		line;
	int				brackets = 0;
	
	if (!configFile.is_open())
		throw configFileException();
	_config.push_back(config());
	while (std::getline(configFile, line))
	{
		try {
			_getConfigLine(line, brackets);
		} catch (std::exception &err)
		{
			std::cout << err.what() << std::endl;
		}
	}
	configFile.close();
}

void	ServerConfig::_getConfigLine(const std::string &str, int &brackets)
{
	std::vector<std::string>	values;
	std::string					key;
	int							i = 0, j = 0;

	while (str[i] && std::isspace(str[i]))
		i++;
	if (str[i] == '}')
	{
		if (brackets > 0)
			brackets--;
		else
		{
			throw configFileBracketsException();
		}
		std::cout << brackets << std::endl;
	}
	if (!str[i] || str[i] == '#')
		return ;
	while (str[i + j] && str[i + j] != ':')
		j++;
	if (j == 0)
		throw configFileLineException();
	key = str.substr(i, j);
	std::cout << "KEY: " << key << " Values: ";
	values = _getConfigValues(str.substr(i + j + 1));
	_validateConfigValues(key, values, brackets);
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
			throw configFileLineException();
		values.push_back(value);
		std::cout << value << "|";
		if (str[i + j] == ';' || (values.size() == 1 && values[values.size() - 1] == "{"))
			break;
		if (!str[i + j])
			throw configFileLineException();
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

void	ServerConfig::_validateConfigValues(const std::string &key, const std::vector<std::string> values, int &brackets)
{
	if (key == "server")
	{
		if (_validateChildValue(values) && brackets == 0)
		{
			_config.push_back(config());
			_config.back().routes.push_back(routes());
			brackets++;
		}
		else
			throw configFileLineException();
	}
	else if (key == "location")
	{
		if (_validateChildValue(values) && brackets == 1)
		{
			_config.back().routes.push_back(routes());
			brackets++;
		}
		else
			throw configFileLineException();
	}
	else
	{
		if (key == "port")
		{
			if (values.size() == 1 && _validateNbr(values[0]))
				_config.back().port = _stoi(values[0]);
		}
	}
}

bool	ServerConfig::_validateChildValue(const std::vector<std::string> values)
{
	return (values.size() == 1 && values[0] == "{");
}

bool	ServerConfig::_validateNbr(const std::string &value)
{
	for (size_t i = 0; i < value.length() - 1; i++)
	{
		if (!std::isdigit(value[i]))
			return false;
	}
	return true;
}

int	ServerConfig::_stoi(const std::string &str)
{
	std::stringstream ss(str);
	int num;
	ss >> num;
	return num;
}
