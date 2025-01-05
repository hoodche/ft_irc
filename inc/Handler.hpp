
#ifndef HANDLER_HPP
# define HANDLER_HPP

#include "../inc/Client.hpp"
#include "../inc/Channel.hpp"
#include "../inc/utils.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>

typedef void (*cmdHandler)(std::string, Client &);

class Handler {
	private:
		std::map<std::string, cmdHandler> cmdMap;
		std::vector<Channel> channels;

		void initCmdMap(void);
		static void handleJoinCmd(std::string input, Client &client); //static cause 
	//it is common to all the instances

	public:
		Handler(void);
		void parseCommand(std::string input, std::vector<Client> &clients, int fd);
};

#endif
