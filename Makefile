NAME = ircserv
SRCS =	
CLFAGS = -Wall -Wextra -Werror -std=c++98
OBJS = $(SRCS:.cpp=.o)

$(NAME): $(OBJS)
	c++ $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	c++ -o $@ -c $< $(CFLAGS)

all: $(NAME)

clean:
	@rm -rf *.o

fclean:
	@rm -rf *.o $(NAME)

re: fclean all

.PHONY: all clean fclean re