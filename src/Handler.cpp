#include "../inc/Handler.hpp"
#include <sstream>

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

std::string Handler::toUpperCase(std::string str) {
    std::string	ret = str;

	for (size_t i = 0; i < str.size(); i++)
		ret[i] = toupper(ret[i]);
	return ret;
}

//JOIN command

void Handler::handleJoinCmd(std::vector<std::string> input, Client &client) {

	std::vector<std::string>			channelVector;
	std::vector<std::string>			passVector;

	if (input.size() <= 1 || input.size() > 3)
	{
		std::cerr << "Invalid JOIN command format" << std::endl;
		return;
	} //Check if we only have JOIN
	std::vector<std::string>::iterator argvIt = input.begin();
	try{
		argvIt++; //Jump over JOIN
		channelVector = getChannelVector(*argvIt);
		argvIt++; //Jump over JOIN
		if (argvIt != input.end())
			passVector = getPassVector(*argvIt);
	}catch(const std::exception &e){
		std::cout << e.what() << std::endl;
	}
}

std::vector<std::string> getChannelVector(std::string channelString)
{
	std::vector<std::string>	channels;
	std::stringstream			ss(string);
	std::string					tempChannel;

	while(std::getline(ss, tempChannel, ','))
	{
		if (*tempChannel.begin() != '#')
			throw std::invalid_argument("Invalidad JOIN command format");
		channels.push_back(tempChannel);
	}
	return (channels);
}

std::vector<std::string> getPassVector(std::string passString)
{
	std::vector<std::string>	passwords;
	std::stringstream			ss(string);
	std::string					tempPass;

	while(std::getline(ss, tempPass, ','))
	{
		if (*tempChannel.begin() == '#')
			throw std::invalid_argument("Invalidad JOIN command format");
		passwords.push_back(tempPass);
	}
	return (passwords);
}

// Verify if the channel exists and call the appropriate function accordingly.
void Handler::joinCmdExec(std::map<std::string, std::string> channelPassDictionary, Client &client)
{
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
