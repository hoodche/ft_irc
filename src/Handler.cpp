#include "../inc/Handler.hpp"
#include <sstream>

std::list<Channel> Handler::channels; //Static variable must be declared outside the class so the linker can fint it. It is not vinculated to an object,
										//so the programmer have to do the job

Handler::Handler(void) {
	//initCmdMap(); // Initialise the command map
}

/*
void Handler::initCmdMap(void) {
	cmdMap["USER"] = &handleUserCmd;
	cmdMap["NICK"] = &handleNickCmd;
	cmdMap["PING"] = &handlePingCmd;
	cmdMap["JOIN"] = &handleJoinCmd;
	cmdMap["TOPIC"] = &handleTopicCmd;
}
*/
void Handler::parseCommand(std::string input, std::list<Client> &clients, int fd){
	(void)input;
	// Protect empty string input -> Check
	if (input.empty())
        throw std::invalid_argument("Empty command received");
	std::string	command;
	Client	*client = Client::findClientByFd(fd, clients);
	if (!client) {
        std::cerr << "Client not found for fd: " << fd << std::endl;
        return;
    }
	std::vector<std::string> realInput;
	realInput.push_back("JOIN");
	realInput.push_back("#betis,#hola,#adios");
	//realInput.push_back("pass1,pass2,pass3,pass4");
	handleJoinCmd(realInput, *client);
	realInput.clear();
	realInput.push_back("TOPIC");
	realInput.push_back("#betis");
	realInput.push_back("hola como estamos");
	handleTopicCmd(realInput, *client);
}
/*

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
*/
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

/*					*/
/*	 JOIN command	*/
/*					*/

void Handler::handleJoinCmd(std::vector<std::string> input, Client &client) {

	std::vector<std::string>				channelVector;
	std::vector<std::string>				passVector;
	std::map<std::string, std::string>		channelDictionary;

	if (input.size() <= 1 || input.size() > 3)
	{
		std::cerr << "Invalid JOIN command format" << std::endl;
		return;
	}
	std::vector<std::string>::iterator argvIt = input.begin();
	try{
		argvIt++;
		channelVector = getChannelVector(*argvIt);
		argvIt++;
		if (argvIt != input.end())
			passVector = getPassVector(*argvIt);
		channelDictionary = createDictionary(channelVector, passVector);
		joinCmdExec(channelDictionary, client);
	}catch(const std::exception &e){
		std::cout << e.what() << std::endl;
	}

}

std::vector<std::string> Handler::getChannelVector(std::string channelString)
{
	std::vector<std::string>	channels;
	std::stringstream			ss(channelString);
	std::string					tempChannel;

	while(std::getline(ss, tempChannel, ','))
	{
		if (*tempChannel.begin() != '#')
			throw std::invalid_argument("Invalidad JOIN command format");
		channels.push_back(tempChannel);
	}
	return (channels);
}

std::vector<std::string> Handler::getPassVector(std::string passString)
{
	std::vector<std::string>	passwords;
	std::stringstream			ss(passString);
	std::string					tempPass;

	while(std::getline(ss, tempPass, ','))
	{
		if (*tempPass.begin() == '#')
			throw std::invalid_argument("Invalidad JOIN command format");
		passwords.push_back(tempPass);
	}
	return (passwords);
}

std::map<std::string, std::string> Handler::createDictionary(std::vector<std::string> &channelVector, std::vector<std::string> &passVector)
{
	std::map<std::string, std::string>		channelDictionary;

	std::vector<std::string>::iterator channelIt = channelVector.begin();
	std::vector<std::string>::iterator passIt = passVector.begin();
	while(channelIt != channelVector.end() && passIt != passVector.end())
	{
		channelDictionary[*channelIt] = *passIt;
		channelIt++;
		passIt++;
	}
	while (channelIt != channelVector.end()){
		channelDictionary[*channelIt] = "";
		channelIt++;
	}
	return (channelDictionary);
}

// Verify if the channel exists and call the appropriate function accordingly.
void Handler::joinCmdExec(std::map<std::string, std::string> channelDictionary, Client &client)
{
	std::map<std::string, std::string>::iterator	itMap = channelDictionary.begin();
	std::list<Channel>::iterator					itChannels;

	while (itMap != channelDictionary.end())
	{
		itChannels = findChannel(itMap->first);
		if (itChannels == channels.end())
			createChannel(itMap->first, client);
		else
			addClientToChannel(*itChannels, client);
		itMap++;
	}
}

std::list<Channel>::iterator Handler::findChannel(const std::string &channelName)
{
	std::list<Channel>::iterator itChannels = channels.begin();

	while (itChannels != channels.end() && itChannels->getName() != channelName)
		itChannels++;
	return (itChannels);
}

void Handler::createChannel(std::string channelName, Client &client)
{
	Channel channel(client);

	channel.setName(channelName);
	channels.push_back(channel);
	client.addChannel(channels.back());
	return;
}

void Handler::addClientToChannel(Channel &channel, Client &client)
{
	channel.addUser(client);
	client.addChannel(channel);
	return;
}

/*					*/
/*	 TOPIC command	*/
/*					*/

void Handler::handleTopicCmd(std::vector<std::string> input, Client &client)
{
	if (input.size() < 2)
	{
		std::cerr << "Incorrect number of arguments" << std::endl;
		return;
	}
	try{
		Channel *targetChannel = client.getChannel(input[1]); //Care, it throws an exception
		if (input.size() == 2)
			std::cout << targetChannel->getTopic() << std::endl;
		else
		{
			std::vector<std::string> vectorTopic(input.begin() + 2, input.end());
			std::string topic = vectorToString(vectorTopic, ' ');
			targetChannel->setTopic(topic, client);
		}
	}catch(std::exception &e){
		std::cerr << e.what() << std::endl;
	}
}

std::string Handler::vectorToString(std::vector<std::string> vectorTopic, char delim)
{
	std::ostringstream	ss;

	std::vector<std::string>::iterator it = vectorTopic.begin();
	while(it != vectorTopic.end()){
		ss << *it;
		if (it != vectorTopic.end() - 1)
			ss << delim;
		it++;
	}
	return (ss.str());
}
