NAME = ircserv

$(NAME):
#	clang++ $(CFLAGS) $(OBJS) -o $(NAME)
	clang++ -g *.cpp -o $(NAME)
#%.o: %.cpp
#	clang++  -g *.cpp -o $@ -c $< $(CFLAGS)

all: $(NAME)

clean:
	@rm -rf *.o

fclean:
	@rm -rf *.o $(NAME)

re: fclean all

.PHONY: all clean fclean re