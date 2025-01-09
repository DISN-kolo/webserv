#ifndef RESPONSE_GENERATOR_HPP
# define RESPONSE_GENERATOR_HPP
# include "webserv.hpp"

# include <sstream>

// wait, what SHOULD a response generator have?
// should it parse requests?

class ResponseGenerator
{
private:
	ResponseGenerator();
	ResponseGenerator(const ResponseGenerator & obj);
	ResponseGenerator &operator=(const ResponseGenerator & obj);

	std::string	_getContent(int code);
	std::string	_getStatusMessage(int status);
	std::string	_getDate(void);
	std::string	_getServerName(void);
	std::string	_getContentType(void);

	std::string	_text;
public:
//	ResponseGenerator(/*some parsed object thing here*/);
	ResponseGenerator(int code);
	~ResponseGenerator();

	std::string	getText(void) const;
	size_t		getSize(void) const;
} ;
#endif
