NAME = WebServ

CPP = c++

FLAGS = -Wall -Werror -g -Wextra -std=c++98

SRC = 	main.cpp \
		srcs/client/Client.cpp \
		srcs/config/GlobalConfig.cpp \
		srcs/config/ServerConfig.cpp \
		srcs/config/LocationConfig.cpp \
		srcs/handlers/Handler.cpp \
		srcs/handlers/HandlerCGI.cpp \
		srcs/parse/ParseConfig.cpp \
		srcs/requests/HttpException.cpp \
		srcs/requests/HttpRequest.cpp \
		srcs/requests/HttpResponse.cpp \
		srcs/server/AServer.cpp \
		srcs/server/EpollServer.cpp \
		srcs/server/PollServer.cpp \
		srcs/server/Socket.cpp \
		srcs/utils/logs.cpp \
		srcs/utils/requestAndResponse.cpp \
		srcs/utils/utils.cpp

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

cleanlog:
		rm ./log/*.log

.PHONY: all clean fclean re