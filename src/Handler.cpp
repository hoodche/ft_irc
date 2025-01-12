#include "../inc/Handler.hpp"

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

/**
 * @brief	handles the irc "USER <username> <hostname> <servername> :<realname>" command
 * @param	std::string input . "USER " was already trimmed
 * @param	Client &client who sent the USER command
 */

void Handler::handleUserCmd(std::string input, Client &client) {
	// Generally seen like this: <username> 0 * <realname> as per this documentation - https://modern.ircdocs.horse/#user-message
	// Default client received command: USER nerea 0 * :realname -> input = nerea 0 * :realname
	std::string errResponse = "USER ";
	errResponse += ERR_NEEDMOREPARAMS;
	std::vector<std::string> params;
	std::istringstream	ss(input);
	std::string	token;

	// Store my tokens in a vector
	while (ss >> token)
		params.push_back(token);

	// Check that we have the 4 required parameters
	if (params.size() < 4) {
		sendResponse(errResponse, client.getSocketFd());
		return ;
	}

	// Assign received strings
	std::string	username = params[0];
	std::string hostname = params[1];
	std::string servername = params[2];
	std::string realname = params[3];

	// Debug print
	// std::cout << "Debug: Split message into words: " << std::endl;
    // for (size_t i = 0; i < params.size(); i++) {
    //     std::cout << "Word " << i + 1 << ": " << params[i] << std::endl;
    // }
	
	// Check that username goes according to what is expected, otherwise send an error to the client
	if (username.empty() || username.length() < 1 || username.length() > USERLEN) {
		sendResponse(errResponse, client.getSocketFd());
		return ;
	}

	// Check that the hostname is as expected
	if (hostname != "0") {
		sendResponse(errResponse, client.getSocketFd());
		return ;
	}

	// Check that the servername is as expected
	if (servername != "*") {
		sendResponse(errResponse, client.getSocketFd());
		return ;
	}

	// Check that realname starts with a :
	if (realname[0] != ':') {
		sendResponse(errResponse, client.getSocketFd());
		return ;
	}
	realname = realname.substr(1);
	client.setUsername(username);
	client.setRealname(realname);
	// std::cout << "Debug print: " << client.getRealname() << std::endl;
	// std::cout << "Username [" << client.getUsername() << "] and realname [" << client.getRealname() << "]" << std::endl;
}

//  TO DO: We will have to keep track of all nicknames, nicknames Cannot repeat
//    The idea of the nickname on IRC is very convenient for users to use
//    when talking to each other outside of a channel, but there is only a
//    finite nickname space and being what they are, it's not uncommon for
//    several people to want to use the same nick.  If a nickname is chosen
//    by two people using this protocol, either one will not succeed or
//    both will removed by use of a server KILL (See Section 3.7.1).
// 	  RFC: 4.1.2 -More info

/**
 * @brief	handles the irc "NICK chosennick" command
 * @param	std::string input . "NICK " was already trimmed
 * @param	Client &client who sent the NICK command
 * 
 */
void Handler::handleNickCmd(std::string input, Client &client) {
	std::string	message;

	if (input.empty()) {
		sendResponse(ERR_NONICKNAMEGIVEN, client.getSocketFd());
		return ;
	}
	if (input.size() < 1 || input.size() > 9) {
		message	= input + " " + ERR_ERRONEUSNICKNAME; // To do: Check if space is needed
		sendResponse(message, client.getSocketFd());
		return ;
	}
	// Check alphanumeric characters
    for (std::string::size_type i = 0; i < input.size(); ++i) {
        if (!std::isalnum(input[i])) {
            message = input + " " + ERR_ERRONEUSNICKNAME;
            sendResponse(message, client.getSocketFd());
            return;
        }
    }
	client.setNickname(input);
	// Debug print
	std::cout << "Client nickname set to: " << input << std::endl;
}

void Handler::handlePingCmd(std::string input, Client &client) {
    std::string message = "PONG :" + input;
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
