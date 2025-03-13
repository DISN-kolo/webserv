NAME = webserv

SRCNAMES = main.cpp \
		   exceptions.cpp \
		   Server.cpp \
		   ServerConfig.cpp \
		   ResponseGenerator.cpp \
		   RequestHeadParser.cpp \
		   Connect.cpp

SRCS = $(addprefix src/, $(SRCNAMES))
OBJS = $(addprefix obj/, $(SRCNAMES:.cpp=.o))
DEPS = $(addprefix obj/, $(SRCNAMES:.cpp=.d))

CC = c++

#CFLAGS = -std=c++98 -O3# -DDEBUG_SERVER_MESSAGES
#CFLAGS = -Wall -Wextra -Werror -std=c++98 -g -DDEBUG_SERVER_MESSAGES
CFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g# -DDEBUG_SERVER_MESSAGES

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
