#ifndef MICROSERV_HPP
# define MICROSERV_HPP

# include <iostream>

class Server ;
class ServerConfig ;
# include "Server.hpp"
# include "ServerConfig.hpp"

# ifndef CRLF
#  define CRLF "\r\n"
# endif

# ifndef RBUF_SIZE
#  define RBUF_SIZE 32000
# endif

# ifndef BLOG_SIZE
#  define BLOG_SIZE 4096
# endif

# include "exceptions.hpp"
#endif
