NAME = webserv

SRCNAMES = main.cpp \
		   exceptions.cpp \
		   Server.cpp \
		   ServerConfig.cpp \
		   ResponseGenerator.cpp \
		   RequestHeadParser.cpp \
		   Connect.cpp
#		   RequestHeadParser.cpp \
#		   RequestBodyParser.cpp

SRCS = $(addprefix src/, $(SRCNAMES))
OBJS = $(addprefix obj/, $(SRCNAMES:.cpp=.o))
DEPS = $(addprefix obj/, $(SRCNAMES:.cpp=.d))

CC = c++

#CFLAGS = -Wall -Wextra -Werror -std=c++98
#CFLAGS = -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address -DDEBUG_SERVER_MESSAGES
# debug print only
#CFLAGS = -Wall -Wextra -Werror -std=c++11 -g -fsanitize=address

RM = rm -rf

.PHONY: all clean fclean re

all: folders $(NAME)

folders:
	@mkdir -p obj

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

obj/%.o: src/%.cpp Makefile
	$(CC) $(CFLAGS) -c $< -MMD -o $@

-include $(DEPS)

clean:
	$(RM) obj

fclean: clean
	$(RM) $(NAME)

re: fclean all
