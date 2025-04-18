#include "../inc/exceptions.hpp"

const char *envException::what(void) const throw()
{
	return ("Enviroment error: unable to read correctly");
}

const char *configFileException::what(void) const throw()
{
	return ("Config file error: unable to open file");
}

const char *configFileLineException::what(void) const throw()
{
	return ("Config file error: bad line");
}

const char *configFileBracketsException::what(void) const throw()
{
	return ("Config file error: bad brackets");
}

const char *configFileMissingException::what(void) const throw()
{
	return ("Config file error: missing configuration");
}

const char *pipeError::what(void) const throw()
{
	return ("pipe failed");
}

const char *execveError::what(void) const throw()
{
	return ("execve failed");
}

const char *socketUnopenedError::what(void) const throw()
{
	return ("Socket error: unable to open");
}

const char *sockOptError::what(void) const throw()
{
	return ("SockOpt error: unable to set");
}

const char *bindError::what(void) const throw()
{
	return ("Bind error");
}

const char *listenError::what(void) const throw()
{
	return ("Can't hear anything");
}

const char *acceptError::what(void) const throw()
{
	return ("This is unacceptable!");
}

const char *badPortError::what(void) const throw()
{
	return ("Bad port (use 1024-65535)");
}

const char *tooManyPorts::what(void) const throw()
{
	return ("Can't have THAT many ports open");
}

const char *readError::what(void) const throw()
{
	return ("Read error");
}

const char *fcntlError::what(void) const throw()
{
	return ("fcntl error (get/set opt, for example)");
}

const char *selectError::what(void) const throw()
{
	return ("Select error");
}

const char *pollError::what(void) const throw()
{
	return ("Poll error");
}

const char *badRequest::what(void) const throw()
{
	return ("400 Bad Request");
}

const char *forbidden::what(void) const throw()
{
	return ("403 Forbidden");
}

const char *notFound::what(void) const throw()
{
	return ("404 Not Found");
}

const char *methodNotAllowed::what(void) const throw()
{
	return ("405 Method Not Allowed");
}

const char *lengthRequired::what(void) const throw()
{
	return ("411 Length Required");
}

const char *contentTooLarge::what(void) const throw()
{
	return ("413 Content Too Large");
}

const char *internalServerError::what(void) const throw()
{
	return ("500 Internal Server Error");
}
