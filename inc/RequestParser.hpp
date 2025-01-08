#ifndef REQUEST_PARSER_HPP
# define REQUEST_PARSER_HPP
# include "webserv.hpp"

class RequestParser
{
private:
	RequestParser();
	RequestParser(const RequestParser & obj);
	RequestParser &operator=(const RequestParser & obj);

	std::string	reqType;
	std::string	askedPath;
public:
	RequestParser(const std::string request);
	~RequestParser();
} ;
#endif
