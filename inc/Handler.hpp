
#ifndef HANDLER_HPP
# define HANDLER_HPP

#include "../inc/Client.hpp"
// #include "../inc/utils.hpp"
#include <sys/socket.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

typedef void (*cmdHandler)(std::string, Client &);

class Handler {
	private:
		std::map<std::string, cmdHandler> cmdMap;
		void initCmdMap(void);

	public:
		Handler(void);
		void parseCommand(std::string input, std::vector<Client> &clients, int fd);

		// Utils
		static void sendResponse(std::string message, int clientFd);
		static std::string toUpperCase(std::string str);

		// Methods for pointers to function
		static void handleUserCmd(std::string input, Client &client);
		static void handleNickCmd(std::string input, Client &client);
};

#endif