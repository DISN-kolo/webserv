#ifndef MICROSERV_HPP
# define MICROSERV_HPP

# include <iostream>
# include <fstream>
# include <iomanip>
# include <map>
# include <vector>
# include <algorithm>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
# include <cstdio>

/*
class Server ;
class ServerConfig ;
# include "Server.hpp"
# include "ServerConfig.hpp"
*/

# define KA_TIME 15
# define CGI_TIME 10
# define CONNS_AMT 500
# ifndef CRLF
#  define CRLF "\r\n"
# endif
// and now some linebreaks that are apparently valid
// this one is probably gonna count for LFLF
# ifndef CRLFLF
#  define CRLFLF "\r\n\n"
# endif
# ifndef LFCRLF
#  define LFCRLF "\n\r\n"
# endif
# ifndef LFLF
#  define LFLF "\n\n"
# endif
// this one is probably gonna count for LFCRLF
# ifndef CRLFCRLF
#  define CRLFCRLF "\r\n\r\n"
# endif

# include "exceptions.hpp"
#endif
