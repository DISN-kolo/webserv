#include "../inc/RequestParser.hpp"

RequestParser::RequestParser()
{
}

RequestParser::RequestParser(const RequestParser & obj)
{
	(void)obj;
}

RequestParser &RequestParser::operator=(const RequestParser & obj)
{
	(void)obj;
	return (*this);
}

RequestParser::RequestParser(const char *r)
	:	_request(r), _reqType("text/html"), _askedPath("/index.html"), _ka(true)
{
}

RequestParser::~RequestParser()
{
}
