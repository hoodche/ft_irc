#include "../inc/Handler.hpp"
#include <algorithm>

Handler::Handler(void) {
	initCmdMap(); // Initialise the command map
}

void Handler::initCmdMap(void) {
	cmdMap["USER"] = &handleUserCmd;
	cmdMap["NICK"] = &handleNickCmd;
	cmdMap["JOIN"] = &handleJoinCmd;
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

//JOIN command

void Handler::handleJoinCmd(std::string input, Client &client) {
	(void)client; //-Werror
	//Find a space to check format
	size_t space = input.find(' ');
    if (space == std::string::npos) {
        std::cerr << "Invalid JOIN command format" << std::endl;
        return;
    }

	//Check if the name of the channel begins with #
    std::string::iterator channelIterator = std::find(input.begin(), input.end(), ' ') + 1;
	while (*channelIterator != '#' && channelIterator != input.end())
		channelIterator++;
	if (*channelIterator != '#'){
        std::cerr << "Invalid JOIN command format" << std::endl;
		return;
	}

	//Get name of the channel
    std::string::iterator beginChCommand = channelIterator;
	while (*channelIterator != ' ' && channelIterator != input.end())
		channelIterator++;
	std::string channelName(beginChCommand, channelIterator);
	if (channelName == "#"){
        std::cerr << "Invalid JOIN command format" << std::endl;
		return;
	} //TODO check ###########
	std::cout << "Correct channel" << std::endl;
	return;
}
