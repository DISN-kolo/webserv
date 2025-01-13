#include "../inc/RequestParser.hpp"
#

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

// lol idk if that's allowed
#include <algorithm>
RequestParser::RequestParser(std::string r)
	:	_r(r)
{
	// bare minimum as per subject
	_acceptableMethods.push_back("GET");
	_acceptableMethods.push_back("POST");
	_acceptableMethods.push_back("DELETE");
	std::string			line;
	std::string			word;
	std::istringstream	s(r);
	std::getline(s, line);
	std::istringstream	ss(line);
	int	i = 0;
	while (std::getline(ss, word, ' '))
	{
		if (i == 0 && std::find(_acceptableMethods.begin(), _acceptableMethods.end(), word) == _acceptableMethods.end())
			throw badRequest();
		i++;
	}
}

RequestParser::~RequestParser()
{
}
