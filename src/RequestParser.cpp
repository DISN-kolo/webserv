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

RequestParser::RequestParser(std::string r)
	:	_r(r)
{
	std::string			line;
	std::string			word;
	std::istringstream	s(r);
	std::getline(s, line);
	std::istringstream	ss(line);
	while (std::getline(ss, word, ' '))
	{
		std::cout << word << std::endl;
	}
}

RequestParser::~RequestParser()
{
}
