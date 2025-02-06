
NAME	= ircserv
FLAGS	= -fsanitize=address -g3 -pedantic -Wall -Wextra -Werror -std=c++98
CXX		= c++
SRCS	= main.cpp src/Server.cpp src/Client.cpp src/Handler.cpp src/Channel.cpp

OBJS = ${SRCS:.cpp=.o}
HEADER_DEPS = ${OBJS:.o=.d}

%.o:%.cpp
	$(CXX) $(FLAGS) -MMD -MP -c $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $(NAME) 
clean:
	rm -rf $(OBJS) $(HEADER_DEPS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(HEADER_DEPS)
