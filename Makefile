NAME = webserv

SRC = seLst.cpp parse.cpp httpRequest.cpp directories.cpp delete_method.cpp clientQueue.cpp server.cpp

OBJ = $(SRC:.cpp=.o)

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $(<:.cpp=.o) -I

all: $(NAME) 

CC		= c++

CCFLAGS	= -Wall -Werror -Wextra -std=c++98

$(NAME): $(OBJ)
		$(CC) $(CCFLAGS) $(OBJ) -o $(NAME)
clean:
		rm -rf $(OBJ)

fclean: clean
		rm -rf $(NAME)

re : fclean all

.PHONY: all clean fclean re
