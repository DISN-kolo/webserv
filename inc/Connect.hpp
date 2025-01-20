#ifndef CONNECT_HPP
# define CONNECT_HPP

# include "webserv.hpp"

// connect needs to know:
// 1. if it needs to continue (this will be known after a req head parse)
// 2. if it does, for how long (as in N of characters)
// 2.1. maybe even the temp body string???
// 3. keep-alive status
// 4. transfer-encoding: chunked (? ? ?)
// it's a solution to handling multiple aux parameters per established connection
class	Connect
{
private:
	Connect(const Connect & obj);
	Connect &operator=(const Connect & obj);

	bool	_needsBody;
	size_t	_contLen;
	bool	_keepAlive;

public:
	Connect();
	~Connect();

	bool	getNeedsBody(void) const;
	size_t	getContLen(void) const;
	bool	getKeepAlive(void) const;

	void	setNeedsBody(bool);
	void	setContLen(size_t);
	void	setKeepAlive(bool);
} ;

#endif
