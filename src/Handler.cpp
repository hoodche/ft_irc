
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
	// Protect empty string input -> Check
	if (input.empty())
        throw std::invalid_argument("Empty command received");
	std::string	command;
	Client	*client = Client::findClientByFd(fd, clients);
	if (!client) {
        std::cerr << "Client not found for fd: " << fd << std::endl;
        return;
    }

	// Remove leading '/' if present when receiving commands -> Check
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

// Pointers to functions methods
void Handler::handleUserCmd(std::string input, Client &client) {
     std::cout << "Handling USER command: [" << input << "]" << std::endl;
	size_t space = input.find(' ');
    if (space == std::string::npos) {
        std::cerr << "Invalid USER command format" << std::endl;
        return;
    }

	std::string username = input.substr(space + 1);
	if (!username.empty() && username[username.size() - 1] == '\n')
        username.erase(username.size() - 1);
    std::string message = ":ircserv 001 " + client.getNickname() + " :Welcome to our IRC server!\n";
    if (client.getUsername() == "") {
    	client.setUsername(username);
        client.setRegistered(true);
		std::cout << "PRINT: " << message << std::endl;
		sendResponse(message, client.getSocketFd());
	}
    std::cout << "Debug print: Client fd " << client.getSocketFd() << " set user to " << username << std::endl;
}

void Handler::handleNickCmd(std::string input, Client &client) {
    std::cout << "Handling NICK command: [" << input << "]" << std::endl;

	size_t space = input.find(' ');
    if (space == std::string::npos) {
        std::cerr << "Invalid NICK command format" << std::endl;
        return;
    }

    std::string nickname = input.substr(space + 1);
	if (!nickname.empty() && nickname[nickname.size() - 1] == '\n') {
        nickname.erase(nickname.size() - 1);
    }
    client.setNickname(nickname);
    std::cout << "Debug print: Client fd " << client.getSocketFd() << " set nickname to " << nickname << std::endl;
}

// Utils
void Handler::sendResponse(std::string message, int clientFd) {
	ssize_t bytesSent = send(clientFd, message.c_str(), message.size(), 0); // Flag 0 = Default behaviour. man send to see further behaviour.
	if (bytesSent == -1) {
		std::cerr << "Failed to send response to client" << std::endl;
	} else {
		// Debug Print
		std::cout << "Response sent to client: " << message << std::endl;
	}
}

std::string Handler::toUpperCase(std::string str) {
    std::string	ret = str;

	for (size_t i = 0; i < str.size(); i++)
		ret[i] = toupper(ret[i]);
	return ret;
}

