#include "../inc/RequestHeadParser.hpp"

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

char	RequestHeadParser::_hexToAscii(size_t i) const
{
	std::string	smallHex = "0123456789abcdef";
	std::string	bigHex = "0123456789ABCDEF";
	size_t		found;
	found = smallHex.find(_rTarget[i + 1]);
	if (found == std::string::npos)
	{
		found = bigHex.find(_rTarget[i + 1]);
	}
	char	result = found * 16;
	found = smallHex.find(_rTarget[i + 2]);
	if (found == std::string::npos)
	{
		found = bigHex.find(_rTarget[i + 2]);
	}
	result += found;
	return (result);
}

// what a name lol.
// resolves the %XX stuff into ascii
// resolves the ../../../../.. situations into presentable paths (can't go above
// the server's "/"
// adds the <location-thing-path>/ to the front
void	RequestHeadParser::_pathDeobfuscator(void)
{
	std::string			allHexes = "0123456789abcdefABCDEF";
	// dubious forbiddenness
//	std::string			forbiddenCharsPrintable = " \"<>\\^`{|}";
	// doing a 0x00 here woudl probably be weird and un-parseable
	char				lowestForbiddenChar = 0x01;
	char				highestForbiddenChar = 0x1F;
	char				bonusForbiddenChar = 0x7F;
	std::ostringstream	oss;
	size_t				i = 0;
	// check for forbidden symbols in raw uri
	// maybe a special kind of error for bad uri?
	for (size_t j = 0; j < _rTarget.size(); j++)
	{
		if (_rTarget[j] == bonusForbiddenChar
			|| (_rTarget[j] <= highestForbiddenChar && _rTarget[j] >= lowestForbiddenChar))
		{
			throw badRequest();
		}
	}
	// convert the %XX
	while (i < _rTarget.size())
	{
		if (_rTarget[i] != '%')
		{
			oss << _rTarget[i];
			i++;
			continue ;
		}
		if (i > _rTarget.size() - 3)
		{
			oss << _rTarget[i];
			i++;
			continue ;
		}
		if (allHexes.find(_rTarget[i + 1]) == std::string::npos
			|| allHexes.find(_rTarget[i + 2]) == std::string::npos)
		{
			oss << _rTarget[i];
			i++;
			continue ;
		}
		// okay, now we're for sure in a %-parsing scenario
		oss << _hexToAscii(i);
		i += 3;
	}
	// parse the / shenanigans. divide by chunks, and if "..", shorten. then, unite again.
	_rTarget = oss.str();
	std::stringstream			ss(_rTarget);
	std::string					pathPart;
	std::vector<std::string>	parsedPath;
	while (std::getline(ss, pathPart, '/'))
	{
		if (pathPart == "" || pathPart == ".")
		{
			continue ;
		}
		else if (pathPart == "..")
		{
			if (!parsedPath.empty())
			{
				parsedPath.pop_back();
			}
		}
		else
		{
			parsedPath.push_back(pathPart);
		}
	}
	_rTarget.clear();
	if (parsedPath.empty())
	{
		_rTarget = "/";
		return ;
	}
	std::vector<std::string>::iterator	it;
	for (it = parsedPath.begin(); it != parsedPath.end(); it++)
	{
		_rTarget += "/" + *it;
	}
}

RequestHeadParser::RequestHeadParser(std::string r)
	:	_r(r)
{
	_defaultContentPath = std::string("/tmp/var/www/") + "/filesforserver";
	_contLen = 0;
	// bare minimum as per subject
	_acceptableMethods.push_back("GET ");
	_acceptableMethods.push_back("POST ");
	_acceptableMethods.push_back("DELETE ");
	_head["connection"] = "";
	// parsing this seems like pain, why not just ka forever by default? okay, 30s if we implement the timer
	_head["keep-alive"] = "";
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
			_method = (*it).substr(0, (*it).size() - 1);
			break ;
		}
		it++;
	}
	if (it == _acceptableMethods.end())
	{
		std::cout << std::setw(7) << " > " << std::flush;
		std::cout << "Method not found in acceptable list" << std::endl;
		// TODO maybe 501?
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
	// solely for apache benchmark purposes xddddddd
	if (line.find("HTTP/1.1\r", line.size() - std::string("HTTP/1.1\r").size()) == std::string::npos)
	{
		if (line.find("HTTP/1.0\r", line.size() - std::string("HTTP/1.0\r").size()) == std::string::npos)
		{
			std::cout << std::setw(7) << " > " << std::flush;
			std::cout << "Bad protocol" << std::endl;
			throw badRequest();
		}
		else
		{
			_protocol = "HTTP/1.0";
		}
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

	_pathDeobfuscator();
	if (_rTarget == std::string("/"))
	{
		_rTarget += "index.html";
	}
	_rTarget = _defaultContentPath + _rTarget;
	// maybe, just maybe, make it accept a string a return a string. maybe for some future use or something. idk. XXX?
	std::cout << "true rtarget: '" << _rTarget << "'" << std::endl;

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
		if (line == "\r" || line == "")
			break ;
		pos = line.find(":");
		if (pos == std::string::npos)
		{
			std::cout << std::setw(7) << " > " << std::flush;
			std::cout << "Didn't find a colon in '" << line.substr(0, line.size() - 1) << "'" << std::endl;
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
			// rm the \r IF it's there
			// if some idiotic protocol decides to plop a naked \n down there to mark like the "new line"
			// we're gonna be like alright it's valid too yay.
			// HOWEVER if someone is like "yoooo it would be awesome to include a \n in one of the fields"
			// we're gonna be like nuh uuuuuuuuuuuh!! nuuuuuuuuuuuuuuuuuuhhhh uuuuuuuuuuuuuuhhh!
			// sorry croski but it's straight to the next field.
			if (line.find("\r") == line.size() - 1)
			{
				_head[helper] = line.substr(pos, line.size() - 1);
			}
			// rm spaces. it's optional tho
			if (*(_head[helper].begin()) == ' ')
				_head[helper].erase(0, _head[helper].find_first_not_of(' '));
			if (*(_head[helper].end() - 1) == ' ')
				_head[helper].erase(_head[helper].find_last_not_of(' ') + 1);
		}
		else
		{
			std::cout << std::setw(7) << " > " << std::flush;
			std::cout << "Unknown line found: '" << line.substr(0, line.size() - 1) << "'" << std::endl;
		}
	}

	// k-a managing, part 1
	helper = _head["connection"];
	std::string::iterator	itt = helper.begin();
	while (itt < helper.end())
	{
		*itt = std::tolower(*itt);
		itt++;
	}
	// default k-a values
	if (_protocol == "HTTP/1.0")
	{
		_keepAlive = false;
		// and now to actually check the header...
		if (helper.find("keep-alive") != std::string::npos)
		{
			_keepAlive = true;
		}
	}
	else
	{
		_keepAlive = true;
		if (helper.find("keep-alive") == std::string::npos)
		{
			if (helper.find("close") != std::string::npos)
			{
				_keepAlive = false;
			}
		}
	}

	// time to manage k-a parameters
	//"keep-alive: thing=value,thing=value"
	// or maybe later TODO
	// keep-alive timeout in seconds by default shall be:
	_kaTimeout = 15;

	// c-l managing
	if (_method == "POST")
	{
		if (_head["content-length"] == "")
		{
			throw lengthRequired();
		}
		std::istringstream	cls(_head["content-length"]);
		cls >> _contLen;
		if (cls.fail() || cls.peek() > 0)
		{
			std::cout << std::setw(7) << " > " << std::flush;
			std::cout << "Content length '" << _head["content-length"] << "' found to be bad" << std::endl;
			throw badRequest();
		}
	}
}

RequestHeadParser::~RequestHeadParser()
{
}

std::map<std::string, std::string>	RequestHeadParser::getHead(void) const
{
	return (_head);
}

std::string	RequestHeadParser::getMethod(void) const
{
	return (_method);
}

std::string	RequestHeadParser::getRTarget(void) const
{
	return (_rTarget);
}

std::string	RequestHeadParser::getProtocol(void) const
{
	return (_protocol);
}

size_t		RequestHeadParser::getContLen(void) const
{
	return (_contLen);
}

bool		RequestHeadParser::getKeepAlive(void) const
{
	return (_keepAlive);
}

time_t		RequestHeadParser::getKaTimeout(void) const
{
	return (_kaTimeout);
}
