
#ifndef HANDLER_HPP
# define HANDLER_HPP

#include "../inc/Client.hpp"
#include "../inc/utils.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>

typedef void (*cmdHandler)(std::string, std::vector<Client>&);

class Handler {
	private:
		std::map<std::string, cmdHandler> cmdMap;
		void initCmdMap(void);

	public:
		Handler(void);
		void parseCommand(std::string input, std::vector<Client> &clients);
};

#endif