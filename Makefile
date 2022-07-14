#COMPILATION
CC			= clang++
CFLAGS		= -g -Wall -Wextra -Werror -std=c++98
.DEFAULT_GOAL := all

#VALUES
NAME		= ircserv
#MISC

NO_COLOR	= \033[1;36m
OK_COLOR	= \033[0;32m
KILL_COLOR	= \033[0;31m
WARN_COLOR	= \033[0;33m

#SOURCES

SRC =		Channel.cpp		\
			Client.cpp 		\
			IRCData.cpp		\
			IRCmsg.cpp		\
			IRCutils.cpp	\
			main.cpp

SRCBONUS =	bot.cpp			\
			IRCmsg.cpp		\
			IRCutils.cpp

$(NAME):
		$(CC) $(SRC) $(CFLAGS) -o $(NAME)


all:		$(NAME)

bonus:		
			$(CC) $(SRCBONUS) $(CFLAGS) -o awesomebot


exe:		all
			./$(NAME)

clean:
			@rm -f *.o; rm -f *~
			@echo "${WARN_COLOR}temporary files removed"
			@echo "${NO_COLOR}"

fclean: clean
				@rm -f $(NAME) awesomebot
				@echo "${KILL_COLOR}exec deleted"
				@echo "${NO_COLOR}"

re:		fclean all

.PHONY: all clean fclean re