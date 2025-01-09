#ifndef REQUEST_PARSER_HPP
# define REQUEST_PARSER_HPP
# include "webserv.hpp"

class RequestParser
{
private:
	RequestParser();
	RequestParser(const RequestParser & obj);
	RequestParser &operator=(const RequestParser & obj);

	std::string	_request;
	std::string	_reqType;
	std::string	_askedPath;
	// keep-alive
	bool		_ka;
public:
	RequestParser(const char *r);
	~RequestParser();
} ;
#endif
