#ifndef REQUEST_PARSER_HPP
# define REQUEST_PARSER_HPP
# include "webserv.hpp"
# include <sstream>
# include <iostream>
# include <string>

class RequestParser
{
private:
	RequestParser();
	RequestParser(const RequestParser & obj);
	RequestParser &operator=(const RequestParser & obj);

	std::vector<std::string>	_acceptableMethods;

	// like, the totality
	std::string	_r;

	// the first lone ever
	std::string	_method;
	std::string	_rTarget;
	std::string	_protocol;

	// a dictionary of the head. if some head line isn't matching, just do a "bad request" :)
	std::map<std::string, std::string>	_head;
public:
	RequestParser(std::string r);
	~RequestParser();
} ;
#endif
