#ifndef RESPONSE_GENERATOR_HPP
# define RESPONSE_GENERATOR_HPP
# include "webserv.hpp"

// wait, what SHOULD a response generator have?
// should it parse requests?

class ResponseGenerator
{
private:
	ResponseGenerator();
	ResponseGenerator(const ResponseGenerator & obj);
	ResponseGenerator &operator=(const ResponseGenerator & obj);
public:
//	ResponseGenerator(const std::string request);
	~ResponseGenerator();
} ;
#endif
