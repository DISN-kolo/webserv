#ifndef WS_EXCEPTIONS_HPP
# define WS_EXCEPTIONS_HPP

# include <exception>

class	configFileException : public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	configFileLineException : public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	configFileBracketsException : public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	configFileMissingException : public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

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


// http errors:
class	badRequest: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	notFound: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	lengthRequired: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

class	internalServerError: public std::exception
{
	public:
		virtual const char	*what(void) const throw();
};

#endif
