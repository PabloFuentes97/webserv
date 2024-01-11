NAME = webserv

SRC = seLst.cpp parse.cpp httpMethods.cpp httpResponse.cpp httpRequest.cpp \
	directories.cpp socketConnection.cpp binary_insert.cpp charptr_n.cpp \
	clientQueue.cpp webserv.cpp

OBJ = $(SRC:.cpp=.o)

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $(<:.cpp=.o) -I

all: $(NAME) 

CC		= c++

CCFLAGS	= -Wall -Werror -Wextra -std=c++98 #-fsanitize=address

$(NAME): $(OBJ)
		$(CC) $(CCFLAGS) $(OBJ) -o $(NAME)
clean:
		rm -rf $(OBJ)

fclean: clean
		rm -rf $(NAME)

re : fclean all

.PHONY: all clean fclean re
