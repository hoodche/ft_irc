#include "../inc/Handler.hpp"
#include "../inc/Server.hpp" // Include the header file for the Server class
#include "../commands/nick.cpp"

std::vector<Channel> Handler::channels; //Static variable must be declared outside the class so the linker can fint it. It is not vinculated to an object,
										//so the programmer have to do the job

Handler::Handler(void) {
	initCmdMap(); // Initialise the command map
}

void Handler::initCmdMap(void) {
	// Estos se hacen en el flujo de auth, a lo mejo es mejor quitarlos de aquí (USER, NICK, PING)
	cmdMap["USER"] = &handleUserCmd;
	cmdMap["NICK"] = &handleNickCmd;
	cmdMap["PING"] = &handlePingCmd;
	cmdMap["JOIN"] = &handleJoinCmd;
}

void Handler::parseCommand(std::vector<std::string> divMsg, Client &client, std::vector<Client> &clients) {
	// Check if the command exists in the map. Command extracted as first member of str vector
	// If command exists and all is good, delete command from str vector
	// Esto solo está puesto para evitar el -Werror de momento
	//--------
	if (divMsg[0] == "NICK") {
		std::cout << "Found nick";
	}
	std::cout << client.getSocketFd() << std::endl;
	std::cout << clients[0].getSocketFd() << std::endl;
	std::cout << "GOT TO PARSE COMMAND! \n\n\n";
	//---------
	// TO DO: Implementar que cada comando vaya a su respectiva función. Código anterior:
	// if (cmdMap.find(command) != cmdMap.end()) {
    //     cmdMap[command](input, *client);
    // } else {
    //     std::cout << "Unknown command: " << command << std::endl;
    // }
	// Pero ahora hay que cambiarlo al tener el vector de strings, etc. 
}

// Pointers to functions methods
void Handler::handleUserCmd(std::string input, Client &client) {
	// To Do: Re-implement User Command
	std::cout << "Client fd: " << client.getSocketFd() << std::endl;
	std::cout << "Received username: " << input << std::endl;
}

//  TO DO: We will have to keep track of all nicknames, nicknames Cannot repeat
//    The idea of the nickname on IRC is very convenient for users to use
//    when talking to each other outside of a channel, but there is only a
//    finite nickname space and being what they are, it's not uncommon for
//    several people to want to use the same nick.  If a nickname is chosen
//    by two people using this protocol, either one will not succeed or
//    both will removed by use of a server KILL (See Section 3.7.1).
// 	  RFC: 4.1.2 -More info

void Handler::handleNickCmd(std::string input, Client &client) {
	if (!input.empty() && !std::isspace(input.at(0)))//we need to discard messages that were "NICKx" since we needed to get here with all messages "NICK" followed by 0 or more whitespaces in order to send a ERR_NONICKNAMEGIVEN response
		return;
	std::string	nickname = Server::trimMessage(input);
	if (nickname.empty()) {
		sendResponse(composeResponse(ERR_NONICKNAMEGIVEN_CODE, ERR_NONICKNAMEGIVEN, "", client.getSocketFd()), client.getSocketFd());
		return ;
	}
	if (!isNicknameValid(nickname))
	{
		sendResponse(composeResponse(ERR_ERRONEUSNICKNAME_CODE, ERR_ERRONEUSNICKNAME, "", client.getSocketFd()), client.getSocketFd());
		return ;
	}
	client.setNickname(nickname);
	// Debug print
	std::cout << "Client nickname set to: >>" << nickname << "<<" << std::endl;
}

void Handler::handlePingCmd(std::string input, Client &client) {
    std::string message = "PONG :" + input;
    sendResponse(message, client.getSocketFd());
}

// Utils
std::string Handler::prependMyserverName(int clientFd) {
	struct sockaddr_in myServerAddr;
	memset(&myServerAddr, 0, sizeof(myServerAddr));
	socklen_t addrLen = sizeof(myServerAddr);
	if (getsockname(clientFd, (struct sockaddr*)&myServerAddr, &addrLen) < 0)
	{
		std::cerr << "Error getting server name to send response" << std::endl;
		return ("");
	}
	return (":" + std::string(inet_ntoa(myServerAddr.sin_addr)) + " ");
}

std::string Handler::composeResponse(std::string field1, std::string field2, std::string field3, int clientFd) {
	std::string message = prependMyserverName(clientFd);
	message += field1 + field2 + field3 +"\n";
	return (message);
}

void Handler::sendResponse(std::string message, int clientFd) {
	ssize_t bytesSent = send(clientFd, message.c_str(), message.size(), 0); // Flag 0 = Default behaviour. man send to see further behaviour.
	if (bytesSent == -1) {
		std::cout << "Failed to send response to client" << std::endl;
	} else {
		// Debug Print
		std::cout << "Response sent to client: " << message << std::endl;
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
