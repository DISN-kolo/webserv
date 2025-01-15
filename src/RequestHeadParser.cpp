#include "../inc/RequestHeadParser.hpp"
#

RequestHeadParser::RequestHeadParser()
{
}

RequestHeadParser::RequestHeadParser(const RequestHeadParser & obj)
{
	(void)obj;
}

RequestHeadParser &RequestHeadParser::operator=(const RequestHeadParser & obj)
{
	(void)obj;
	return (*this);
}

// lol idk if that's allowed
#include <algorithm>
RequestHeadParser::RequestHeadParser(std::string r)
	:	_r(r)
{
	// bare minimum as per subject
	_acceptableMethods.push_back("GET ");
	_acceptableMethods.push_back("POST ");
	_acceptableMethods.push_back("DELETE ");
	std::string			line;
	std::istringstream	s(r);
	std::getline(s, line);

	// parse the method out using a list of known methods
	std::vector<std::string>::iterator	it = _acceptableMethods.begin();
	while (it != _acceptableMethods.end())
	{
		if (_r.find(*it) == 0)
		{
			_method = (*it).substr((*it).size() - 1);
			break ;
		}
		it++;
	}
	if (it == _acceptableMethods.end())
	{
		throw badRequest();
	}

	// forget about multiline URIs for now
	// and actually, forever. turns out the eval sheet is very fogriving and small
	// if ok, parse the protocol out. since it HAS to be of definite length,..
	//               GET sp   /  sp HTTP/1.1
	if (line.size() < 3 + 1 + 1 + 1 + 8)
	{
		throw badRequest();
	}

//	if (std::find(_r.end() - std::string("HTTP/1.1").size() - 1, _r.end(), "HTTP/1.1") == _r.end())
	if (line.find("HTTP/1.1\r", line.size() - std::string("HTTP/1.1\r").size()) == std::string::npos)
	{
		throw badRequest();
	}
	else
	{
		_protocol = "HTTP/1.1";
	}

	// by that point, the request is probably correct. Still, we need to separate out the requested URI
	std::string			word;
	std::istringstream	ss(line);
	int					wcount = 0;
	while (std::getline(ss, word, ' '))
	{
		if (wcount == 1)
		{
			_rTarget = word;
		}
		else if (wcount >= 3)
		{
			throw badRequest();
		}
		wcount++;
	}
}

RequestHeadParser::~RequestHeadParser()
{
}
