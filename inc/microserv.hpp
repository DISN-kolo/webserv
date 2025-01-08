#ifndef MICROSERV_HPP
# define MICROSERV_HPP

# ifndef CRLF
#  define CRLF "\r\n"
# endif

# ifndef RBUF_SIZE
#  define RBUF_SIZE 32000
# endif

# ifndef BLOG_SIZE
#  define BLOG_SIZE 4096
# endif

# include <iostream>
# include <iomanip>
# include <sstream>
# include <ctime>
# include <exception>
# include <unistd.h>
# include <poll.h>
# include <sys/types.h>
# include <sys/stat.h>
//# include <netinet/in.h>
# include <arpa/inet.h>
# include <cstdlib>
# include <cstring>
# include <fcntl.h>
# include <unistd.h>
# include <cerrno>
# include <csignal>

class	socketUnopenedError : public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	sockOptError: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	bindError : public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	listenError : public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	acceptError: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	badPortError: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	tooManyPorts: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	readError: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	fcntlError: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	selectError: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	pollError: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

#endif
