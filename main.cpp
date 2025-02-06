
#include <iostream>
#include <cstring>
#include <sstream> // for stringstream
#include "inc/Server.hpp"
#include <csignal> //for SIGINT and SIGQUIT

/**
 * @brief		converts a string to an integer (since ++98 standard has no 
 * 				functions to convert string to numeric types we must do our own
 * 				 with string streams)
 * @param		const std::string& str .string to convert
 * @return		int converted number
 */
int myStoi(const std::string& str)
{
	int num;
	std::stringstream ss(str);
	ss >> num;
	return num;
}

/**
 * @brief		checks if arguments passed to the program are 3, if the second
 * 				is a correct port number with just digits and the third is a 
 * 				password with less than 21 characters
 * @param		int argc number of arguments passed to the program
 * @param		char **argv arguments passed to the program
 * @return		int 0 on error, 1 on success
 */
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
		if(myStoi(argv[1]) < 1024 || myStoi(argv[1]) > 65535 )
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

// Main function
int main(int argc, char **argv)
{
	if (!argsOk(argc, argv))
		return 1;
	Server server;
	signal(SIGINT, Server::signalHandler);
	signal(SIGQUIT, Server::signalHandler);
	try
	{
		server.init(myStoi(argv[1]), static_cast<std::string>(argv[2]));
	}
	catch(const std::exception& e)
	{
		server.closeFds();
		std::cout << e.what() << std::endl;
	}
}
