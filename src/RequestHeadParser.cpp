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
	_head["connection"] = "keep-alive";
	// parsing this seems like pain, why not just ka forever by default? okay, 30s if we implement the timer
//	_head["keep-alive"] = "";
	_head["content-length"] = "";
	// who cares
//	_head["content-type"] = "";
	// now THIS might be important. or not
//	_head["content-encoding"] = "";
	// ok this IS important. config supports multiple hosts i assume.
	_head["host"] = "";
	// chunking. if you wanna...
//	_head["transfer-encoding"] = "";
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
		std::cout << std::setw(7) << " > " << std::flush;
		std::cout << "Method not found in acceptable list" << std::endl;
		throw badRequest();
	}

	// forget about multiline URIs for now
	// and actually, forever. turns out the eval sheet is very fogriving and small
	// if ok, parse the protocol out. since it HAS to be of definite length,..
	//               GET sp   /  sp HTTP/1.1
	if (line.size() < 3 + 1 + 1 + 1 + 8)
	{
		std::cout << std::setw(7) << " > " << std::flush;
		std::cout << "This first line is way too short" << std::endl;
		throw badRequest();
	}

	// HTTP/1.0 support or nah? XXX
	if (line.find("HTTP/1.1\r", line.size() - std::string("HTTP/1.1\r").size()) == std::string::npos)
	{
		std::cout << std::setw(7) << " > " << std::flush;
		std::cout << "Bad protocol" << std::endl;
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
			std::cout << std::setw(7) << " > " << std::flush;
			std::cout << "Too many words in Start Line" << std::endl;
			throw badRequest();
		}
		wcount++;
	}

	// amazing, first line parsed.
	// now, 1. get next line
	// 2. check for ':'
	// 3. grab everything before it
	// 4. lowercase
	// 5. match to map
	// 5.1 if not in map = do nothing :) :P >w< ^^ :3 c: :D
	// 5.2 if in map = grab the following value from space (space is optional) until '\r'
	size_t		pos;
	std::string	helper;
	while (std::getline(s, line))
	{
		if (line == "\r")
			break ;
		pos = line.find(":");
		if (pos == std::string::npos)
		{
			std::cout << std::setw(7) << " > " << std::flush;
			std::cout << "Didn't find a semi-colon in '" << line.substr(0, line.size() - 1) << "'" << std::endl;
			throw badRequest();
		}
		helper = line.substr(0, pos);
		if (helper.size() <= 1)
		{
			std::cout << std::setw(7) << " > " << std::flush;
			std::cout << "Semicolon encountered without field name? on '" << line.substr(0, line.size() - 1) << "'" << std::endl;
			throw badRequest();
		}
		if (helper.find(" ") != std::string::npos)
		{
			std::cout << std::setw(7) << " > " << std::flush;
			std::cout << "Spaces in field name on '" << line.substr(0, line.size() - 1) << "'" << std::endl;
			throw badRequest();
		}
		std::string::iterator	itt = helper.begin();
		while (itt < helper.end())
		{
			*itt = std::tolower(*itt);
			itt++;
		}
		if (_head.find(helper) != _head.end())
		{
			//                                           - 1 to rm the \r
			_head[helper] = line.substr(pos, line.size() - 1);
		}
		else
		{
			std::cout << std::setw(7) << " > " << std::flush;
			std::cout << "Unknown line found: '" << line.substr(0, line.size() - 1) << "'" << std::endl;
		}
	}
}

RequestHeadParser::~RequestHeadParser()
{
}
