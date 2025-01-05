
#include "../inc/utils.hpp"

// Utils function
// Used to turn command to uppder case since, in irssi, both lowercase and uppercase commands work
std::string	toUpperCase(std::string str) {
	std::string	ret = str;

	for (size_t i = 0; i < str.size(); i++)
		ret[i] = toupper(ret[i]);
	return ret;
}

// Handler class parser functions
// void handleCapCmd(std::string input, std::vector<Client> &clients) {
// 	std::cout << "Handling CAP auth connection" << std::endl;
// 	// CAP LS 302 is present everytime the irssi client connects. CAP LS basically looks for the server capabilities. 
// 	if (input.find("LS 302") != std::string::npos) {
//         std::cout << "Handling CAP LS 302" << std::endl;
//         std::string message = "CAP * LS :multi-prefix";
// 		sendResponse(message, clients[0].getSocketFd()); // Question: I'm confused as to how to send the specific fd of the especific client. Testing with client[0] for now
//     }
// 	// Debug prints, uncomment to get further info
// 	// std::cout << "Parsed command "<< input << std::endl;
// 	// std::cout << "Client fd:" << clients[0].getSocketFd() << std::endl;
// }

void handleNickCmd(std::string input, Client &client) {
	size_t space = input.find(' ');
    if (space == std::string::npos) {
        std::cerr << "Invalid NICK command format" << std::endl;
        return;
    }

    std::string nickname = input.substr(space + 1);
	if (!nickname.empty() && nickname[nickname.size() - 1] == '\n') {
        nickname.erase(nickname.size() - 1);
    }
	if (client.getNickname() == "")
    	client.setNickname(nickname);
    // std::cout << "Debug print: Client fd " << client.getSocketFd() << " set nickname to " << nickname << std::endl;
}

void handleUserCmd(std::string input, Client &client) {
	size_t space = input.find(' ');
    if (space == std::string::npos) {
        std::cerr << "Invalid USER command format" << std::endl;
        return;
    }

	std::string username = input.substr(space + 1);
	if (!username.empty() && username[username.size() - 1] == '\n') {
        username.erase(username.size() - 1);
    }
    std::string message = ":ircserv 001 " + client.getNickname() + " :Welcome to our IRC server!\n";
	if (client.getUsername() == "") {
    	client.setUsername(username);
		std::cout << "PRINT: " << message << std::endl;
		sendResponse(message, client.getSocketFd());
	}
    // std::cout << "Debug print: Client fd " << client.getSocketFd() << " set user to " << username << std::endl;
}

// void handlePingCmd() {
// 	std::string inpi
// }

void sendResponse(std::string message, int clientFd) {
	ssize_t bytesSent = send(clientFd, message.c_str(), message.size(), 0); // Flag 0 = Default behaviour. man send to see further behaviour.
	if (bytesSent == -1) {
		std::cerr << "Failed to send response to client" << std::endl;
	} else {
		// Debug Print
		std::cout << "Response sent to client: " << message << std::endl;
	}
}
