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

	// the first line ever
	std::string	_method;
	std::string	_rTarget;
	std::string	_protocol;

	// a dictionary of the head. if some head line isn't matching, just do a "bad request" :)
	std::map<std::string, std::string>	_head;

	// since we're gonna parse it anyways to check for the correctness of the field,
	// why not store it for later use, to avoid parsing it twice.
	size_t		_contLen;
	bool		_keepAlive;
	time_t		_kaTimeout;
public:
	RequestHeadParser(std::string r);
	~RequestHeadParser();

	std::map<std::string, std::string>	getHead(void) const;
	std::string	getMethod(void) const;
	std::string	getRTarget(void) const;
	std::string	getProtocol(void) const;

	size_t		getContLen(void) const;

	bool		getKeepAlive(void) const;
	time_t		getKaTimeout(void) const;
} ;
#endif
