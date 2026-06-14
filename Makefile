NAME = WebServ

CPP = c++

FLAGS = -Wall -Werror -g -Wextra -std=c++98

SRC = \
		main.cpp \
		srcs/server/Socket.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME) : $(OBJ)
		$(CPP) $(FLAGS) $(OBJ) -o $@

%.o : %.cpp
		$(CPP) $(FLAGS) -c $< -o $@

clean:
		rm -f $(OBJ)
fclean: clean
		rm -f $(NAME)
re: fclean all

.PHONY: all clean fclean re