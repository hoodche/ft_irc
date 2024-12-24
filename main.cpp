/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igcastil <igcastil@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 13:18:44 by igcastil          #+#    #+#             */
/*   Updated: 2024/12/23 18:55:23 by igcastil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
bool isPortValid(std::string port){
	return (port.find_first_not_of("0123456789") == std::string::npos && \
	std::atoi(port.c_str()) >= 1024 && std::atoi(port.c_str()) <= 65535);
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "execute: ircserv <port number> <password>" << std::endl;
		return 1;
	} 
	try
	{
		if (static_cast<std::string>(argv[1]).find_first_not_of("0123456789") == std::string::npos)

		if(!isPortValid(av[1]) || !*av[2] || std::strlen(av[2]) > 20)
			{std::cout << "invalid Port number / Password!" << std::endl; return 1;}
		ser.init(std::atoi(av[1]), av[2]);
	}
	catch(const std::exception& e)
	{
		ser.close_fds();
		std::cerr << e.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
}
