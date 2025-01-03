# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: igcastil <igcastil@student.42madrid.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/28 14:26:37 by igcastil          #+#    #+#              #
#    Updated: 2025/01/03 16:39:50 by igcastil         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= ircserv
CC		= c++
FLAGS	= -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME):
	$(CC) $(FLAGS) main.cpp src/Server.cpp -o $(NAME)

client:
	$(CC) $(FLAGS) test_client.cpp -o client
clean:
	rm -rf $(NAME)

fclean:
	rm -rf $(NAME)
	rm -rf client

re: clean all

.PHONY: all clean fclean re