NAME = WebServ

CPP = c++

FLAGS = -Wall -Werror -g -Wextra -std=c++98

SRC = \
		main.cpp \
		srcs/server/Socket.cpp \
		srcs/server/AServer.cpp \
		srcs/server/PollServer.cpp \
		srcs/server/EpollServer.cpp \
		srcs/utils/requestAndResponse.cpp \
		srcs/config/GlobalConfig.cpp \
		srcs/config/ServerConfig.cpp \
		srcs/config/LocationConfig.cpp \
		srcs/client/Client.cpp \
		srcs/requests/HttpRequest.cpp \
		srcs/parse/ParseConfig.cpp \
		srcs/utils/utils.cpp \
		srcs/utils/logs.cpp \
		srcs/requests/HttpException.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME) : $(OBJ)
		$(CPP) $(FLAGS) $(OBJ) -o $@
		./config/generate-config.sh

%.o : %.cpp
		$(CPP) $(FLAGS) -c $< -o $@

clean:
		rm -f $(OBJ)
fclean: clean
		rm -f $(NAME)
		rm -rf config/configs
re: fclean all

.PHONY: all clean fclean re