# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: nvillalt <nvillalt@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/28 14:26:37 by igcastil          #+#    #+#              #
#    Updated: 2025/01/03 22:34:46 by nvillalt         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= ircserv
CC		= c++
FLAGS	= -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME):
	$(CC) $(FLAGS) main.cpp src/utils.cpp src/Server.cpp src/Client.cpp src/Handler.cpp src/Channel.cpp inc/utils.hpp inc/Server.hpp inc/Client.hpp inc/Handler.hpp inc/Channel.hpp -o $(NAME)

client:
	$(CC) $(FLAGS) test_client.cpp -o client
clean:
	rm -rf $(NAME)

fclean:
	rm -rf $(NAME)
	rm -rf client

re: clean all

.PHONY: all clean fclean re
