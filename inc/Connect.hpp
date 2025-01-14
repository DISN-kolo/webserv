#ifndef CONNECT_HPP
# define CONNECT_HPP

// connect needs to know:
// 1. its local recv buffer
// 2. if there's a head received correctly
// 3. if it needs to continue (this will be known after a req head parse)
// 4. if it does, for how long (?)
// 5. keep-alive status
// 6. transfer-encoding: chunked (? ? ?)
// it's a solution to handling multiple aux parameters per established connection
// previously, we had "_localRecvBuffers" in Server do one part of it
class	Connect
{
public:
	Connect();
	~Connect();

	std::string	getLRB(void) const;
	void		setLRB(std::string newBuffer);
	void		clearLRB(void);
private:
	Connect(const Connect & obj);
	Connect &operator=(const Connect & obj);

	std::string	_localRecvBuffer;
	bool		_headOK;
	bool		_needsBody;
	bool		_keepAlive;
} ;

#endif
