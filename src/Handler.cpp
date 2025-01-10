#include "../inc/Handler.hpp"

std::vector<Channel> Handler::channels; //Static variable must be declared outside the class so the linker can fint it. It is not vinculated to an object,
										//so the programmer have to do the job

Handler::Handler(void) {
	initCmdMap(); // Initialise the command map
}

void Handler::initCmdMap(void) {
	cmdMap["USER"] = &handleUserCmd;
	cmdMap["NICK"] = &handleNickCmd;
	cmdMap["PING"] = &handlePingCmd;
	cmdMap["JOIN"] = &handleJoinCmd;
}

void Handler::parseCommand(std::vector<std::string> divMsg, Client &client, std::vector<Client> &clients) {
	// Check if the command exists in the map. Command extracted as first member of str vector
	// If command exists and all is good, delete command from str vector
	if (divMsg[0] == "NICK") {
		std::cout << "Found nick";
	}
	std::cout << client.getSocketFd() << std::endl;
	std::cout << clients[0].getSocketFd() << std::endl;
	std::cout << "GOT TO PARSE COMMAND! \n\n\n";
}

// Pointers to functions methods
void Handler::handleUserCmd(std::string input, Client &client) {
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
		//std::cout << "PRINT: " << message << std::endl;
		sendResponse(message, client.getSocketFd());
	}
    //std::cout << "Debug print: Client fd " << client.getSocketFd() << " set user to " << username << std::endl;
}

void Handler::handleNickCmd(std::string input, Client &client) {
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
    //std::cout << "Debug print: Client fd " << client.getSocketFd() << " set nickname to " << nickname << std::endl;
}

void Handler::handlePingCmd(std::string input, Client &client) {
    size_t space = input.find(' ');
    if (space == std::string::npos) {
        std::cerr << "Invalid NICK command format" << std::endl;
        return;
    }

    std::string pongResponse = input.substr(space + 1);
    	if (!pongResponse.empty() && pongResponse[pongResponse.size() - 1] == '\n') {
        pongResponse.erase(pongResponse.size() - 1);
    }

    std::string message = "PONG " + pongResponse;
    sendResponse(message, client.getSocketFd());
}

// Utils
void Handler::sendResponse(std::string message, int clientFd) {
	ssize_t bytesSent = send(clientFd, message.c_str(), message.size(), 0); // Flag 0 = Default behaviour. man send to see further behaviour.
	if (bytesSent == -1) {
		std::cout << "Failed to send response to client" << std::endl;
	} else {
		// Debug Print
		std::cout << "Response sent to client: " << message << std::endl;
	}
}

// std::string Handler::toUpperCase(std::string str) {
//     std::string	ret = str;

// 	for (size_t i = 0; i < str.size(); i++)
// 		ret[i] = toupper(ret[i]);
// 	return ret;
// }

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
	while (*channelIterator == ' ')
		channelIterator++;
	if (*channelIterator != '#'){
        std::cerr << "Invalid JOIN command format" << std::endl;
		return;
	}

	//Get name of the channel in channelName
    std::string::iterator beginChCommand = channelIterator;
	while (*channelIterator != ' ' && channelIterator != input.end())
		channelIterator++;
	std::string channelName(beginChCommand, channelIterator);

	//Check that we have something more than hastags
	channelIterator = channelName.begin();
	while (*channelIterator == '#')
		channelIterator++;
	if (channelIterator == channelName.end())
        std::cerr << "Invalid JOIN command format" << std::endl;
	else
		joinCmdExec(channelName, client);
	return;
}

// Verify if the channel exists and call the appropriate function accordingly.
void Handler::joinCmdExec(std::string channelName, Client &client)
{
	if (client.isClientInChannel(channelName))
	{
		std::cout << "entra" << std::endl;
		return;
	}
	std::vector<Channel>::iterator chIt = channels.begin();
	while (chIt != channels.end() && chIt->getName() != channelName) //Checks if the channel exists
		chIt++;
	if (chIt != channels.end())
		addClientToChannel(*chIt, client); //Add if the channel exits and add client as user
	else
		createChannel(channelName, client); //Creates the channelxº and add client as operator
}

void Handler::createChannel(std::string channelName, Client &client)
{
	Channel channel(client);

	channel.setName(channelName);
	channels.push_back(channel);
	client.addClientChannel(channels.back());
	return;
}

void Handler::addClientToChannel(Channel &channel, Client &client)
{
	channel.addUser(client);
	client.addClientChannel(channel);
	return;
}
