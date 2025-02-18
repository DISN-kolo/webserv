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
	std::stringstream	ss;
	std::string	cont = _getErrorPage(std::string(ewhat), server);
	ss << "HTTP/1.1 " << ewhat << CRLF;
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

ResponseGenerator::ResponseGenerator(const RequestHeadParser & req)
{
	// let's say that the location directive is already resolved somewhere prior. here,
	if (req.getMethod() == "GET")
	{
		std::cout << "it's a get of '" << req.getRTarget().c_str() << "'" << std::endl;
		struct stat	st;
		int			statResponse;
		statResponse = stat(req.getRTarget().c_str(), &st);
		if (statResponse == -1)
		{
			std::cout << "it's a stat == -1" << std::endl;
			throw notFound();
		}
		if ((st.st_mode & S_IFREG) != S_IFREG)
		{
			std::cout << "it's an st_mode & S_IFREG != S_IFREG" << std::endl;
			std::cout << st.st_mode << std::endl;
			throw internalServerError();
		}
		_fd = open(req.getRTarget().c_str(), O_RDONLY);
		if (_fd == -1)
		{
			std::cout << "it's an fd == -1" << std::endl;
			throw internalServerError();
		}

		_fSize = st.st_size;
		_hasFile = true;
		std::cout << "epic! you've just opened a file to READ. its REAL path is " << req.getRTarget() << ", and the fd is " << _fd << ", and the size is " << _fSize << "." << std::endl;

		std::stringstream	ss;
		ss << "HTTP/1.1 " << "200 OK" << CRLF;
		ss << "Date: " << _getDate() << CRLF;
		ss << "Server: " << _getServerName() << CRLF;
		ss << "Content-Type: " << _getContentType() << CRLF;
		ss << "Content-Length: " << _fSize << CRLF;
		ss << CRLF;
		_text = ss.str();
	}
	else if (req.getMethod() == "POST")
	{
		std::cout << "it's a post of '" << req.getRTarget().c_str() << "'" << std::endl;
		struct stat	st;
		int			statResponse;
		statResponse = stat(req.getRTarget().c_str(), &st);
		if (statResponse != -1)
		{
			// FIXME update existing file instead? idk lol
			std::cout << "it's a stat != -1: file exists already. idk" << std::endl;
			throw internalServerError();
		}
		_fd = open(req.getRTarget().c_str(), O_CREAT | O_WRONLY, 0644);
		if (_fd == -1)
		{
			std::cout << "it's an fd == -1" << std::endl;
			throw internalServerError();
		}

		std::cout << "epic! you've just opened a file to WRITE. its REAL path is " << req.getRTarget() << ", and the fd is " << _fd << "." << std::endl;

		std::stringstream	ss;
		ss << "HTTP/1.1 " << "201 Created" << CRLF;
		ss << "Date: " << _getDate() << CRLF;
		ss << "Server: " << _getServerName() << CRLF;
		ss << CRLF;
		_text = ss.str();
	}
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

// I beg you, make a map.
// or not,, that's why it aint a todo lol
std::string	ResponseGenerator::_getErrorPage(std::string ewhat, struct config_server_t server)
{
	std::string	ret;
	ret = "<head></head><body><p align=\"center\">" + ewhat + "</p></body>";
	return (ret);
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

int	ResponseGenerator::getFd(void) const
{
	return (_fd);
}

bool	ResponseGenerator::getHasFile(void) const
{
	return (_hasFile);
}
