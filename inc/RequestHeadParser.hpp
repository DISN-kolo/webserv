#ifndef REQUEST_HEAD_PARSER_HPP
# define REQUEST_HEAD_PARSER_HPP
# include "webserv.hpp"
# include <sstream>
# include <iostream>
# include <string>

class RequestHeadParser
{
private:
	RequestHeadParser();
	RequestHeadParser(const RequestHeadParser & obj);
	RequestHeadParser &operator=(const RequestHeadParser & obj);

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
	RequestHeadParser(std::string r);
	~RequestHeadParser();
} ;
#endif
