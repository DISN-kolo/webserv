#ifndef MICROSERV_HPP
# define MICROSERV_HPP

# ifndef CRLF
#  define CRLF "\r\n"
# endif

# ifndef RBUF_SIZE
#  define RBUF_SIZE 32000
# endif

# ifndef BLOG_SIZE
#  define BLOG_SIZE 4096
# endif

# include <iostream>
# include <iomanip>
# include <sstream>
# include <ctime>
# include <unistd.h>
# include <poll.h>
# include <sys/types.h>
# include <sys/stat.h>
//# include <netinet/in.h>
# include <arpa/inet.h>
# include <cstdlib>
# include <cstring>
# include <fcntl.h>
# include <unistd.h>
# include <cerrno>
# include <csignal>

# include "exceptions.hpp"
#endif
