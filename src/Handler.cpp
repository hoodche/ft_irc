
#include "../inc/Handler.hpp"

Handler::Handler(void) {
	initCmdMap(); // Initialise the command map
}

void Handler::initCmdMap(void) {
	// cmdMap["JOIN"] = &handleJoinCmd;
	cmdMap["CAP"] = &handleCapCmd;
	// cmdMap["NICK"] = &handleNickCmd;
}

void Handler::parseCommand(std::string input, std::vector<Client> &clients){
	// Protect empty string input
	if (input.empty())
        throw std::invalid_argument("Empty command received");
	std::string	command;
	// Remove leading '/' if present
    if (!input.empty() && input[0] == '/') {
        input = input.substr(1);
    }

	size_t space = input.find_first_of(" \t\n"); // Find the command until space, tab or enter, make it CAPs

	// We extract the first command minus / to compare it in our pointers to function structure
	if (space == std::string::npos)
		command	= toUpperCase(input);
	else
		command	= toUpperCase(input.substr(0, space));
	// Debug prints, uncomment to get further info
	// std::cout << "Parsed command "<<  command << std::endl;
	// std::cout << "Client fd:" << clients[0].getSocketFd() << std::endl;
	    // Check if the command exists in the map
    if (cmdMap.find(command) != cmdMap.end()) {
        // Call the corresponding function using the function pointer
        cmdMap[command](input, clients);
    } else {
        std::cout << "Unknown command: " << command << std::endl;
    }
}

