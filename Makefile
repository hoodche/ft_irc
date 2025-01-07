# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: igcastil <igcastil@student.42madrid.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/28 14:26:37 by igcastil          #+#    #+#              #
#    Updated: 2025/01/07 15:55:24 by igcastil         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= ircserv
CC		= c++
FLAGS	= -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME):
	$(CC) $(FLAGS) main.cpp src/utils.cpp src/Server.cpp src/Client.cpp src/Handler.cpp src/Channel.cpp -o $(NAME)

client:
	$(CC) $(FLAGS) test_client.cpp -o client
clean:
	rm -rf $(NAME)

fclean:
	rm -rf $(NAME)
	rm -rf client

re: clean all

.PHONY: all clean fclean re
