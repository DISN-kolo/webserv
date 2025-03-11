/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseGenerator.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akozin <akozin@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/04 15:59:48 by akozin            #+#    #+#             */
/*   Updated: 2025/03/11 12:34:08 by akozin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ResponseGenerator.hpp"

ResponseGenerator::ResponseGenerator()
{
}

// more or less a placeholder.
ResponseGenerator::ResponseGenerator(int code)
{
	std::stringstream	ss;
	std::string	cont = _getContent(code);
	ss << "HTTP/1.1 " << code << " " << _getStatusMessage(code) << CRLF;
	ss << "Date: " << _getDate() << CRLF;
	ss << "Server: " << _getServerName() << CRLF;
	ss << "Content-Type: " << _getContentType() << CRLF;
	ss << "Content-Length: " << cont.size() << CRLF;
	ss << CRLF;
	ss << cont;
	// no vvvv ? FIXME remove it?
	ss << CRLF;
	_text = ss.str();
}

// this is for errors only. thus we need to check if any of the errors are actually like pre-made pages.
// for that, we need context
ResponseGenerator::ResponseGenerator(const char * ewhat, struct config_server_t server)
{
	_hasFile = false;
	_pid = -2;
	std::string	errFilePath = "";
	std::string	errDirPath = "";
	bool		errFileMatched = false;
	bool		errDirMatched = false;
	std::string	numericCode = std::string(ewhat);
	numericCode = numericCode.substr(0, 3);
#ifdef DEBUG_SERVER_MESSAGES
	std::cout << "numeric code gotten: '" << numericCode << "'" << std::endl;
#endif
	// go thru the config's custom errors. search for the path.
	for (size_t i = 0; i < server.customErrors.size(); i++)
	{
#ifdef DEBUG_SERVER_MESSAGES
		std::cout << "| " << server.customErrors[i].first << " | " << server.customErrors[i].second << " |" << std::endl;
#endif
		if (server.customErrors[i].first == numericCode)
		{
			errFilePath = server.customErrors[i].second;
			errFileMatched = true;
			break ;
		}
		else if (server.customErrors[i].first == "")
		{
			errDirPath = server.customErrors[i].second;
			errDirMatched = true;
		}
	}
	if (errFileMatched || errDirMatched)
	{
		if (!errFileMatched)
		{
			errFilePath = errDirPath + "/" + numericCode + ".html";
		}
#ifdef DEBUG_SERVER_MESSAGES
		std::cout << "it's an error type \"get\" of '" << errFilePath << "'" << std::endl;
#endif
		struct stat	st;
		int			statResponse;
		statResponse = stat(errFilePath.c_str(), &st);
		if (statResponse == -1)
		{
			// no file. send the regular thing.
#ifdef DEBUG_SERVER_MESSAGES
			std::cout << "it's a stat == -1" << std::endl;
#endif
			_defaultErrorPageGenerator(ewhat);
			return ;
		}
		if ((st.st_mode & S_IFREG) != S_IFREG)
		{
			// not a correct kind of file. send the regular thing.
#ifdef DEBUG_SERVER_MESSAGES
			std::cout << "it's a s_ifreg != s_ifreg" << std::endl;
#endif
			_defaultErrorPageGenerator(ewhat);
			return ;
		}
		_fd = open(errFilePath.c_str(), O_RDONLY);
		if (_fd == -1)
		{
			// regular open failed, great. send the regular thing.
#ifdef DEBUG_SERVER_MESSAGES
			std::cout << "it's an fd == -1" << std::endl;
#endif
			_defaultErrorPageGenerator(ewhat);
			return ;
		}

		_fSize = st.st_size;
		_hasFile = true;
#ifdef DEBUG_SERVER_MESSAGES
		std::cout << "error file opened for reading at fd " << _fd << ", and the size is " << _fSize << "." << std::endl;
#endif

		std::stringstream	ss;
		ss << "HTTP/1.1 " << ewhat << CRLF;
		ss << "Date: " << _getDate() << CRLF;
		ss << "Server: " << _getServerName() << CRLF;
		ss << "Content-Type: " << _getContentType() << CRLF;
		ss << "Content-Length: " << _fSize << CRLF;
		ss << CRLF;
		_text = ss.str();
	}
	else
	{
		_hasFile = false;
#ifdef DEBUG_SERVER_MESSAGES
		std::cout << "absolutely default error page initiated" << std::endl;
#endif
		_defaultErrorPageGenerator(ewhat);
	}
}

ResponseGenerator::ResponseGenerator(const RequestHeadParser & req, struct config_server_t server, std::vector<std::string> env)
{
	_hasFile = false;
	_bodyStr = "";
	_pid = -2;
	_server = server;
	_env = env;
	if (req.getDirlist())
	{
		std::string	listing = _generateListing(req.getRTarget(), req.getApparentTarget(), server);
		_hasFile = false;
		std::stringstream	ss;
		ss << "HTTP/1.1 " << "200 OK" << CRLF;
		ss << "Date: " << _getDate() << CRLF;
		ss << "Server: " << _getServerName() << CRLF;
		ss << "Content-Type: " << _getContentType() << CRLF;
		ss << "Content-Length: " << listing.size() << CRLF;
		ss << CRLF;
		ss << listing;
		_text = ss.str();
		return ;
	}
	if (req.getRedirection())
	{
		_hasFile = false;
		std::stringstream	ss;
		ss << "HTTP/1.1 " << "308 Permanent Redirect" << CRLF;
		ss << "Location: " << req.getRedirLoc() << CRLF;
		ss << "Date: " << _getDate() << CRLF;
		ss << "Server: " << _getServerName() << CRLF;
		ss << CRLF;
		_text = ss.str();
		return ;
	}
	// let's say that the location directive is already resolved somewhere prior. here,
	if (req.getMethod() == "GET")
	{
#ifdef DEBUG_SERVER_MESSAGES
		std::cout << "it's a get of '" << req.getRTarget().c_str() << "'" << std::endl;
#endif

		if (req.getIsCgi())
		{
			_fd = _execCgi(req.getRTarget());
			_hasFile = true;
		}
		else
		{
			struct stat	st;
			int			statResponse;
			statResponse = stat(req.getRTarget().c_str(), &st);
			if (statResponse == -1)
			{
#ifdef DEBUG_SERVER_MESSAGES
				std::cout << "it's a stat == -1" << std::endl;
#endif
				throw notFound();
			}
			if ((st.st_mode & S_IFREG) != S_IFREG)
			{
#ifdef DEBUG_SERVER_MESSAGES
				std::cout << "it's an st_mode & S_IFREG != S_IFREG" << std::endl;
#endif
#ifdef DEBUG_SERVER_MESSAGES
				std::cout << st.st_mode << std::endl;
#endif
				throw internalServerError();
			}
			_fd = open(req.getRTarget().c_str(), O_RDONLY);
			_fSize = st.st_size;
			_hasFile = true;
#ifdef DEBUG_SERVER_MESSAGES
			std::cout << "epic! you've just opened a file to READ. its REAL path is " << req.getRTarget() << ", and the fd is " << _fd << ", and the size is " << _fSize << "." << std::endl;
#endif

			std::stringstream	ss;
			ss << "HTTP/1.1 " << "200 OK" << CRLF;
			ss << "Date: " << _getDate() << CRLF;
			ss << "Server: " << _getServerName() << CRLF;
			ss << "Content-Type: " << _getContentType() << CRLF;
			ss << "Content-Length: " << _fSize << CRLF;
			ss << CRLF;
			_text = ss.str();
			if (_fd == -1)
			{
#ifdef DEBUG_SERVER_MESSAGES
				std::cout << "it's an fd == -1" << std::endl;
#endif
				throw internalServerError();
			}
		}
	}
	else if (req.getMethod() == "POST")
	{
#ifdef DEBUG_SERVER_MESSAGES
		std::cout << "it's a post of '" << req.getRTarget().c_str() << "'" << std::endl;
#endif
		struct stat	st;
		int			statResponse;
		statResponse = stat(req.getRTarget().c_str(), &st);
		if (statResponse != -1)
		{
			// FIXME update existing file instead? idk lol
			// TODO add appropriate response codes insteada of just 502!!! this is a MUST have feature before release since it's the correctness of post handling
			throw internalServerError();
		}

		_fd = open(req.getRTarget().c_str(), O_CREAT | O_WRONLY, 0644);
		if (_fd == -1)
		{
			throw internalServerError();
		}

		std::stringstream	ss;
		ss << "HTTP/1.1 " << "201 Created" << CRLF;
		ss << "Date: " << _getDate() << CRLF;
		ss << "Server: " << _getServerName() << CRLF;
		ss << "Content-Location: " << req.getUrl() << CRLF;
		ss << CRLF;
		_text = ss.str();
	}
	else if (req.getMethod() == "DELETE")
	{
#ifdef DEBUG_SERVER_MESSAGES
		std::cout << "it's a delete of '" << req.getRTarget().c_str() << "'" << std::endl;
#endif
		struct stat	st;
		int			statResponse;
		statResponse = stat(req.getRTarget().c_str(), &st);
		if (statResponse == -1)
		{
#ifdef DEBUG_SERVER_MESSAGES
			std::cout << "it's a stat == -1" << std::endl;
#endif
			throw notFound();
		}
		if ((st.st_mode & S_IFREG) != S_IFREG)
		{
#ifdef DEBUG_SERVER_MESSAGES
			std::cout << "it's an st_mode & S_IFREG != S_IFREG" << std::endl;
			std::cout << st.st_mode << std::endl;
#endif
			throw internalServerError();
		}
		if (remove(req.getRTarget().c_str()) != 0)
		{
#ifdef DEBUG_SERVER_MESSAGES
			std::cout << "remove failed" << std::endl;
#endif
			throw internalServerError();
		}
		else
		{
#ifdef DEBUG_SERVER_MESSAGES
			std::cout << "remove OK" << std::endl;
#endif
		}

		std::stringstream	ss;
		ss << "HTTP/1.1 " << "204 No Content" << CRLF;
		ss << "Date: " << _getDate() << CRLF;
		ss << "Server: " << _getServerName() << CRLF;
		ss << "Content-Length: " << 0 << CRLF;
		ss << CRLF;
		_text = ss.str();
	}
}

ResponseGenerator::ResponseGenerator(std::string body, std::string rTarget, std::vector<std::string> env)
{
	_env = env;
	_bodyStr = body;
	_fd = _execCgi(rTarget);
	_hasFile = true;
}

ResponseGenerator::ResponseGenerator(const ResponseGenerator & obj)
{
	(void)obj;
}

ResponseGenerator &ResponseGenerator::operator=(const ResponseGenerator & obj)
{
	(void)obj;
	return (*this);
}

ResponseGenerator::~ResponseGenerator()
{
}

void	ResponseGenerator::_defaultErrorPageGenerator(const char * ewhat)
{
	std::stringstream	ss;
	std::string	cont = "<body><p align=\"center\">" + std::string(ewhat) + "</p></body>";
	ss << "HTTP/1.1 " << ewhat << CRLF;
	ss << "Date: " << _getDate() << CRLF;
	ss << "Server: " << _getServerName() << CRLF;
	ss << "Content-Type: " << _getContentType() << CRLF;
	ss << "Content-Length: " << cont.size() << CRLF;
	ss << CRLF;
	ss << cont;
	ss << CRLF;
	_text = ss.str();
}

std::string	ResponseGenerator::_generateListing(std::string dirpath, std::string apparentTarget, struct config_server_t server)
{
	std::string	ret;
	ret = "<body>\n<h1>\n";
	ret += "Listing of " + apparentTarget + ":\n";
	ret += "</h1>\n<hr />\n<table>\n";
	ret += "<thead>\n<tr>\n<th scope=\"col\">Type</th>\n<th scope=\"col\">Name</th>\n</tr>\n</thead>";
	ret += "<tbody>\n";
	
	DIR				*dirVar = opendir(dirpath.c_str());
	struct dirent	*dEnt;
	if (dirVar == NULL)
	{
#ifdef DEBUG_SERVER_MESSAGES
		std::cout << "opendir failed on autoindexing of " << apparentTarget << ", which is " << dirpath << " in real life" << std::endl;
#endif
		throw internalServerError();
	}

	struct stat	st;
	int			statResponse;
	std::string	fmode;
	dEnt = readdir(dirVar);
	while (dEnt != NULL)
	{
		statResponse = stat((dirpath + "/" + dEnt->d_name).c_str(), &st);
		if (statResponse == -1)
		{
#ifdef DEBUG_SERVER_MESSAGES
			std::cout << "stat failed on " << dirpath << " " << dEnt->d_name << std::endl;
#endif
			dEnt = readdir(dirVar);
			continue ;
		}
		if ((st.st_mode & S_IFREG) == S_IFREG)
		{
			fmode = "file";
		}
		else if ((st.st_mode & S_IFDIR) == S_IFDIR)
		{
			fmode = " dir";
		}
		else
		{
			fmode = "misc";
		}
		std::ostringstream	returlss;
		returlss << "http://" + server.host + ":" << server.ports[0] << apparentTarget + "/";
		ret += "<tr>\n<td>" + fmode + "</td>\n<td><a href=\"" + returlss.str() + std::string(dEnt->d_name) + "\">" + std::string(dEnt->d_name) + "</a></td>\n</tr>\n";
		dEnt = readdir(dirVar);
	}
	ret += "</tbody>\n</table>\n</body>\n";
	return (ret);
}

char	**ResponseGenerator::_generateEnv(void)
{
	int	j = 0;

	if (!_bodyStr.empty())
	{
		for (size_t i = 0; i < _bodyStr.size(); i++)
		{
			if (_bodyStr[i] == '&')
			{
				_env.push_back(_bodyStr.substr(j, i));
				j = i + 1;
			}
			else if (i + 1 == _bodyStr.size())
				_env.push_back(_bodyStr.substr(j, i + 1));
		}
	}

	char	**env = new char*[_env.size() + 1];

	j = 0;
	for (std::vector<std::string>::iterator i = _env.begin(); i != _env.end(); i++)
	{
		env[j] = new char[i->size() + 1];
		env[j++] = const_cast<char *>(i->c_str());
	}
	env[j] = NULL;
	return env;
}

int	ResponseGenerator::_execCgi(std::string rTarget)
{
	pid_t		pid;
	int			fds[2];
	std::string	cgiPath = "/bin/php-cgi", reqRTarget = rTarget;
	char		*argv[3] = {(char *)cgiPath.c_str(), (char *)reqRTarget.c_str(), NULL};
	char		**env;

	if (pipe(fds) < 0)
		throw internalServerError();
	pid = fork();
	if (pid < 0)
		throw internalServerError();
	if (!pid)
	{
		dup2(fds[1], STDOUT_FILENO);
		close(fds[0]);
		close(fds[1]);
		env = _generateEnv();
		execve("/bin/php-cgi", argv, env);
		delete []env;
		throw execveError();
	}
	_pid = pid;
	return (fds[0]);
}

// dummy function. TODO
// real function is probably like:
// 200? get something that was required.
// anything else? get a standard eror page OR a page specified by the config.
// => needs access to config
std::string	ResponseGenerator::_getContent(int code)
{
	if (code == 200)
		return ("<head>Hello, my wonderful friends!</head><body><p>This is an html body :3</p></body>");
	else if (code == 404)
		return ("<head align=\"center\">404 Not Found</head>");
	else
		return ("A secret third option");
}

// TODO proably a good idea to make a map of statuses
std::string	ResponseGenerator::_getStatusMessage(int status)
{
	switch (status)
	{
		case 200:
			return ("OK");
		default:
			return ("Status code message not implemented.");
	}
}

std::string	ResponseGenerator::_getDate(void)
{
	std::stringstream	ss;
	time_t	curTime = time(NULL);
	std::string	days[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	std::string	months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "July", "Aug", "Sep", "Oct", "Nov", "Dec"};
	std::string	ret;
	struct tm	dateTime = *gmtime(&curTime);
	ss << days[dateTime.tm_wday] << ", ";
	if (dateTime.tm_mday < 10)
		ss << "0";
	ss << dateTime.tm_mday << " ";
	ss << months[dateTime.tm_mon] << " ";
	ss << (dateTime.tm_year + 1900) << " ";
	if (dateTime.tm_hour < 10)
		ss << "0";
	ss << dateTime.tm_hour << ":";
	if (dateTime.tm_min < 10)
		ss << "0";
	ss << dateTime.tm_min << ":";
	if (dateTime.tm_sec < 10)
		ss << "0";
	ss << dateTime.tm_sec;
	ret = ss.str();
	return (ret);
}

// needs to access the config file for the servname.
std::string	ResponseGenerator::_getServerName(void)
{
	if (!(_server.name.empty()))
	{
		return (_server.name);
	}
	return ("localhost");
}

// it probably needs to look ath the request and behave appropriately
std::string	ResponseGenerator::_getContentType(void)
{
	return ("text/html");
}

std::string	ResponseGenerator::getText(void) const
{
	return (_text);
}

size_t	ResponseGenerator::getSize(void) const
{
	return (_text.size());
//	if (!_hasFile)
//	{
//		return (_text.size());
//	}
//	else
//	{
//		return (_text.size() + _fSize);
//	}
}

off_t	ResponseGenerator::getFSize(void) const
{
	return (_fSize);
}

int		ResponseGenerator::getFd(void) const
{
	return (_fd);
}

bool	ResponseGenerator::getHasFile(void) const
{
	return (_hasFile);
}

int		ResponseGenerator::getPid(void) const
{
	return (_pid);
}
