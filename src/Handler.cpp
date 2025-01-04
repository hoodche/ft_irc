
#include "../inc/Handler.hpp"

Handler::Handler(void) {
	initCmdMap(); // Initialise the command map
}

void Handler::initCmdMap(void) {
	// cmdMap["JOIN"] = &handleJoinCmd;
	cmdMap["USER"] = &handleUserCmd;
	cmdMap["NICK"] = &handleNickCmd;
	// cmdMap["PING"] = &handlePingCmd;
}

void Handler::parseCommand(std::string input, std::vector<Client> &clients, int fd){
	// Protect empty string input
	if (input.empty())
        throw std::invalid_argument("Empty command received");
	std::string	command;
	Client	*client = Client::findClientByFd(fd, clients);
	if (!client) {
        std::cerr << "Client not found for fd: " << fd << std::endl;
        return;
    }

	// Remove leading '/' if present when receiving commands -> REVIEW!!
    if (!input.empty() && input[0] == '/') {
        input = input.substr(1);
    }

	// Check if the input starts with "CAP LS 302" and skip it
    std::string capLsCmd = "CAP LS 302";
    if (input.rfind(capLsCmd, 0) == 0) { // rfind(capLsCmd, 0) checks if input starts with capLsCmd
        size_t nextCmdStart = input.find_first_of("\n") + 1;
        if (nextCmdStart < input.size()) {
            input = input.substr(nextCmdStart); // Skip CAP LS 302 and focus on the next command
        } else {
            return; // If nothing comes after CAP LS 302, return
        }
    }

	size_t space = input.find_first_of(" \t\n"); // Find the command until space, tab or enter, make it CAPs
	// We extract the first command minus / to compare it in our pointers to function structure
	if (space == std::string::npos)
		command	= toUpperCase(input);
	else
		command	= toUpperCase(input.substr(0, space));
	// Check if the command exists in the map
    if (cmdMap.find(command) != cmdMap.end()) {
        cmdMap[command](input, *client);
    } else {
        std::cout << "Unknown command: " << command << std::endl;
    }
}

