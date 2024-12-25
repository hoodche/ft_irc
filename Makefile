# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: igcastil <igcastil@student.42madrid.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/28 14:26:37 by igcastil          #+#    #+#              #
#    Updated: 2024/12/25 23:56:05 by igcastil         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= ircserv
CC		= c++
FLAGS	= -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME):
	$(CC) $(FLAGS) main.cpp src/Server.cpp -o $(NAME)
clean:
	rm -rf $(NAME)

fclean:
	rm -rf $(NAME)

re: clean all

.PHONY: all clean fclean re