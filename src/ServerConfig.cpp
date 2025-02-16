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
	int				brackets = 0, i = 0;
	
	if (!configFile.is_open())
		throw configFileException();
	while (std::getline(configFile, line))
	{
		i++;
		try {
			_parseLine(line, brackets);
		} catch (std::exception &err)
		{
			std::cout << "Line " << i << ": " << err.what() << std::endl;
		}
	}
	configFile.close();
	_validateConfigRequirements();
}

void	ServerConfig::_parseLine(const std::string &line, int &brackets)
{
	std::string					validLine, key;
	std::vector<std::string>	values;

	validLine = _getLine(line);
	if (validLine.empty())
		return;
	if (validLine[0] == '}')
	{
		_validateBracketClose(validLine, brackets);
		return;
	}
	if (validLine[validLine.length() - 1] != ';' && validLine[validLine.length() - 1] != '{')
		throw configFileLineException();
	key = _getLineKey(validLine);
	values = _getLineValues(validLine);

	_validateConfigValues(key, values, brackets);
}

std::string	ServerConfig::_getLine(const std::string &line)
{
	int	i = 0, j = 0;

	while (line[i] && std::isspace(line[i]))
		i++;
	while (line[i + j] && line[i + j] != '#')
		j++;
	if (!j)
		return ("");
	j--;
	while (std::isspace(line[i + j]))
		j--;
	return (line.substr(i, j + 1));
}

std::string	ServerConfig::_getLineKey(const std::string &line)
{
	int	i = 0;

	while (line[i] && line[i] != ':')
		i++;
	i--;
	while (i > 0 && std::isspace(line[i]))
		i--;
	return (line.substr(0, i + 1));
}

std::vector<std::string>	ServerConfig::_getLineValues(const std::string &line)
{
	std::vector<std::string>	values;
	std::string					value;
	int							i = 0;

	while (line[i] && line[i] != ':')
		i++;
	std::stringstream			str(line.substr(i + 1));
	while (getline(str, value, ','))
		values.push_back(_getLineValue(value));
	return (values);
}

std::string	ServerConfig::_getLineValue(const std::string &value)
{
	int	i = 0, j = 0;

	while (value[i] && std::isspace(value[i]))
		i++;
	while (value[i + j])
		j++;
	j--;
	while (j > 0 && std::isspace(value[i + j]))
		j--;
	if (!j && value[i + j] == ';')
		throw configFileLineException();
	if (value[i + j] == ';')
		j--;
	return (value.substr(i, j + 1));
}



void	ServerConfig::_validateConfigValues(const std::string &key, const std::vector<std::string> values, int &brackets)
{
	if (key == "server")
	{
		if (_validateChildValue(values) && brackets == 0)
		{
			_config.push_back(config());
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
		if (!brackets)
			throw configFileLineException();
		if (key == "port")
			_validatePort(values, brackets);
		else if (key == "host")
			_validateHost(values, brackets);
		else if (key == "server_name")
			_validateServerName(values, brackets);
		else if (key == "error_page")
			_validateErrorPage(values, brackets);
		else if (key == "client_max_body_size")
			_validateMaxBody(values, brackets);
		else if (key == "root")
			_validateRoot(values, brackets);
		else if (key == "index")
			_validateIndex(values, brackets);
		else if (key == "autoindex")
			_validateAutoindex(values, brackets);
		else if (key == "allow_methods")
			_validateMethods(values, brackets);
		else if (key == "name")
			_validateName(values, brackets);
		else if (key == "alias")
			_validateAlias(values, brackets);
		else if (key == "cgi_path")
			_validateCgiPath(values, brackets);
		else if (key == "cgi_enabled")
			_validateCgiEnable(values, brackets);	
		else if (key == "return")
			_validateReturn(values, brackets);
		else
		{
			std::cout << "NULL KEY " << key << std::endl;
			throw configFileLineException();
		}

	}
}

bool	ServerConfig::_validateChildValue(const std::vector<std::string> values)
{
	return (values.size() == 1 && values[0] == "{");
}

void	ServerConfig::_validateBracketClose(const std::string line, int &brackets)
{
	int i = 0;

	while (std::isspace(line[i]))
		i++;
	if (line[i] != ';' && line[i] != '}')
		throw configFileLineException();
	brackets--;
}

void	ServerConfig::_validateAutoindex(std::vector<std::string> values, int brackets)
{
	bool	autoIndex;
	if (values.size() != 1)
		throw configFileLineException();
	if (values[0] == "on")
		autoIndex = true;
	else if (values[0] == "off")
		autoIndex = false;
	else
		throw configFileLineException();
	if (brackets == 1)
		_config.back().autoIndex = autoIndex;
	else
		_config.back().routes.back().autoIndex = autoIndex;
}

void	ServerConfig::_validateMaxBody(std::vector<std::string> values, int brackets)
{
	std::string	u;
	int			i = 0, n;

	if (values.size() != 1)
		throw configFileLineException();
	while (std::isdigit(values[0][i]))
		i++;
	n = _stoi(values[0].substr(0, i));
	u = values[0].substr(i);
	if (u == "GB")
		n *= 1024 * 1024 * 1024;
	else if (u == "MB")
		n *= 1024 * 1024;
	else if (u == "KB")
		n *= 1024;
	else if (!u.empty())
		throw configFileLineException();
	if (brackets == 1)
		_config.back().maxBodySize = n;
	else
		_config.back().routes.back().maxBodySize = n;
}

void	ServerConfig::_validateHost(std::vector<std::string> values, int brackets)
{
	std::string	str;
	int			n;

	if (values.size() != 1 || brackets == 2)
		throw configFileLineException();
	std::stringstream			sstr(values[0]);
	while (getline(sstr, str, '.'))
	{
		for (int i = 0; str[i]; i++)
		{
			if (!std::isdigit(str[i]))
				throw configFileLineException();
		}
		n = _stoi(str);
		if (n < 0 || n > 255)
			throw configFileLineException();
	}
	_config.back().host = values[0];
}

void	ServerConfig::_validateServerName(std::vector<std::string> values, int brackets)
{
	if (values.size() != 1 || brackets == 2)
		throw configFileLineException();
	for (int i = 0; values[0][i]; i++)
	{
		if (values[0][i] == '/')
			throw configFileLineException();
	}
	_config.back().name = values[0];
}

void	ServerConfig::_validateRoot(std::vector<std::string> values, int brackets)
{
	if (values.size() != 1)
		throw configFileLineException();
	if (brackets == 1)
		_config.back().root = values[0];
	else
		_config.back().routes.back().root = values[0];
}

void	ServerConfig::_validateIndex(std::vector<std::string> values, int brackets)
{
	if (values.size() != 1)
		throw configFileLineException();
	if (brackets == 1)
		_config.back().root = values[0];
	else
		_config.back().routes.back().root = values[0];
}

void	ServerConfig::_validateName(std::vector<std::string> values, int brackets)
{
	if (values.size() != 1 || brackets == 1)
		throw configFileLineException();
	_config.back().routes.back().root = values[0];
}

void	ServerConfig::_validateAlias(std::vector<std::string> values, int brackets)
{
	if (values.size() != 1 || brackets == 1)
		throw configFileLineException();
	_config.back().routes.back().alias = values[0];
}

void	ServerConfig::_validatePort(std::vector<std::string> values, int brackets)
{
	std::vector<int>	ports;
	int					n;

	if (brackets != 1)
		throw configFileLineException();
	for (std::vector<std::string>::iterator i = values.begin(); i != values.end(); i++)
	{
		for (size_t j = 0; j < (*i).length() - 1; j++)
		{
			if (!std::isdigit((*i)[j]))
				throw configFileLineException();
		}
		n = _stoi(*i);
		if (n >= 1024 && n <= 65535)
			ports.push_back(n);
		else
			throw configFileLineException();
	}
	_config.back().ports = ports;
}

void	ServerConfig::_validateMethods(std::vector<std::string> values, int brackets)
{
	for (std::vector<std::string>::iterator i = values.begin(); i != values.end(); i++)
	{
		if (*i != "GET" && *i != "POST" && *i != "DELETE")
			throw configFileLineException();
	}

	if (brackets == 1)
		_config.back().accMethods = values;
	else
		_config.back().routes.back().accMethods = values;
}

void	ServerConfig::_validateCgiPath(std::vector<std::string> values, int brackets)
{
	if (brackets == 1)
		_config.back().cgiPath = values;
	else
		_config.back().routes.back().cgiPath = values;
}

void	ServerConfig::_validateCgiEnable(std::vector<std::string> values, int brackets)
{
	bool	enable;

	if (values.size() != 1)
		throw configFileLineException();
	if (values[0] == "true")
		enable = true;
	else if (values[0] == "false")
		enable = false;
	else
		throw configFileLineException();

	if (brackets == 1)
		_config.back().cgiEnable = enable;
	else
		_config.back().routes.back().cgiEnable = enable;
}

void	ServerConfig::_validateReturn(std::vector<std::string> values, int brackets)
{
	if (values.size() != 1 || brackets == 1)
		throw configFileLineException();

	_config.back().routes.back().redir = values[0];
}

void	ServerConfig::_validateErrorPage(std::vector<std::string> values, int brackets)
{
	std::vector<std::pair<std::string, std::string> >	errors;
	std::pair<std::string, std::string>					tmp;
	std::string	errCode, errPath;
	int			j = 0, n;

	if (brackets == 2)
		throw configFileLineException();

	for (std::vector<std::string>::iterator i = values.begin(); i != values.end(); i++)
	{
		if (std::isdigit((*i)[0]))
		{
			while ((*i)[j] && !std::isspace((*i)[j]))
				j++;
			if (j != 3)
				throw configFileLineException();
			errCode = (*i).substr(0, j);
			n = _stoi(errCode);
			if (n < 400 || n > 599)
				throw configFileLineException();
			tmp.first = errCode;
			tmp.second = (*i).substr(j + 2);
		}
		else
		{
			tmp.first = "";
			tmp.second = *i;
		}
		errors.push_back(tmp);
	}

	_config.back().customErrors = errors;
}

void	ServerConfig::_validateConfigRequirements()
{
	for (std::vector<struct config>::iterator i = _config.begin(); i != _config.end(); i++)
	{
		std::cout << "Server " << i->name << " {" << std::endl;
		for (std::vector<struct routes>::iterator j = i->routes.begin(); j != i->routes.end(); j++)
			std::cout << "	Location " << j->name << " { }" << std::endl;
		std::cout << "}" << std::endl;
	}

}

int	ServerConfig::_stoi(const std::string &str)
{
	std::stringstream ss(str);
	int num;
	ss >> num;
	return num;
}
