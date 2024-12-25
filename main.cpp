/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igcastil <igcastil@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 13:18:44 by igcastil          #+#    #+#             */
/*   Updated: 2024/12/25 23:31:05 by igcastil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include <cstdlib> // for atoi
#include "inc/Server.hpp"

int	argsOk(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "execute: ircserv <port number> <password>" << std::endl;
		return 0;
	}
	try
	{
		if (static_cast<std::string>(argv[1]).find_first_not_of("0123456789") != std::string::npos)
		{
			std::cout << "port number only with digits please" << std::endl;
			return 0;
		}
		if(atoi(argv[1]) < 1024 || atoi(argv[1]) > 65535 )
		{
			std::cout << "port number must be >= 1024 and <= 65535" << std::endl;
			return 0;
		}
		if(strlen(argv[2]) > 20 )
		{
			std::cout << "password must not have more than 20 characters" << std::endl;
			return 0;
		}
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 0;
	}
	return 1;
}
int main(int argc, char **argv)
{
	if (!argsOk(argc, argv))
		return 1;
	Server server;
	try
	{
		server.init(atoi(argv[1]), static_cast<std::string>(argv[2]));
	}
	catch(const std::exception& e)
	{
		server.closeFds();
		std::cout << e.what() << std::endl;
	}
}
