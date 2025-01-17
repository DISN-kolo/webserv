#include "../inc/ResponseGenerator.hpp"

ResponseGenerator::ResponseGenerator()
{
}

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

ResponseGenerator::ResponseGenerator(const char * ewhat)
{
	std::stringstream	ss;
	std::string	cont = _getErrorPage(std::string(ewhat));
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
std::string	ResponseGenerator::_getErrorPage(std::string ewhat)
{
	std::string	ret;
	ret = "<head align=\"center\">" + ewhat + "</head>";
	return (ret);
}

// TODO proably a good idea to make a map of statuses
std::string	ResponseGenerator::_getStatusMessage(int status)
{
	switch (status)
	{
		case 200:
			return ("OK");
		case 404:
			return ("Not Found");
		default:
			return ("I'm a teapot");
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
}
