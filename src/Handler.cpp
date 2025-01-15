#include "../inc/Handler.hpp"
#include <sstream>
#include "../inc/Server.hpp" 
#include "../commands/nick.cpp" 

std::list<Channel> Handler::channels; //Static variable must be declared outside the class so the linker can fint it. It is not vinculated to an object,
										//so the programmer have to do the job

Handler::Handler(void) {
	initCmdMap(); // Initialise the command map
}

void Handler::initCmdMap(void) {
	cmdMap["USER"] = &handleUserCmd;
	cmdMap["NICK"] = &handleNickCmd;
	cmdMap["PING"] = &handlePingCmd;
	cmdMap["JOIN"] = &handleJoinCmd;
	cmdMap["TOPIC"] = &handleTopicCmd;
	//cmdMap["KICK"] = &handleKickCmd;
}

void Handler::parseCommand(std::vector<std::string> divMsg, Client &client, std::list<Client> &clients) {
	// Check if the command exists in the map. Command extracted as first member of str vector
	// If command exists and all is good, delete command from str vector
	// Esto solo está puesto para evitar el -Werror de momento
	std::string	command = divMsg[0];
	std::cout << client.getSocketFd() << std::endl;
	std::cout << clients.front().getSocketFd() << std::endl;
	std::cout << "GOT TO PARSE COMMAND! \n\n\n";
	//---------
	// TO DO: Implementar que cada comando vaya a su respectiva función. Código anterior:
	if (cmdMap.find(command) != cmdMap.end()) {
		cmdMap[command](divMsg, client);
    } else {
        std::cout << "" << command << std::endl;
    }
}

/**
 * @brief	handles the irc "USER <username> <hostname> <servername> :<realname>" command
 * @param	std::string input . "USER " was already trimmed
 * @param	Client &client who sent the USER command
 */

void Handler::handleUserCmd(std::vector<std::string> divMsg, Client &client) {
	// Generally seen like this: <username> 0 * <realname> as per this documentation - https://modern.ircdocs.horse/#user-message
	// Default client received command: USER nerea 0 * :realname -> input = nerea 0 * :realname
	// Check that we have the 4 required parameters	
	if (divMsg.size() < 4) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
		return ;
	}

	// Assign received strings
	std::string	username = divMsg[1];
	std::string hostname = divMsg[2];
	std::string servername = divMsg[3];
	std::string realname = "";

	int	vecSize	= divMsg.size();
	for (int i = 4; i < vecSize; i++) {
		realname += divMsg[i];
		if (i < vecSize - 1)
			realname += " ";
	}
	// Debug print
	// std::cout << "Debug: Split message into words: " << std::endl;
    // for (size_t i = 0; i < divMsg.size(); i++) {
    //     std::cout << "Word " << i + 1 << ": " << divMsg[i] << std::endl;
    // }
	
	// Check that username goes according to what is expected, otherwise send an error to the client
	if (username.empty() || username.length() < 1 || username.length() > USERLEN) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
		return ;
	}

	// Check that the hostname is as expected
	if (hostname != "0") {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
		return ;
	}

	// Check that the servername is as expected
	if (servername != "*") {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
		return ;
	}

	// Check that realname starts with a : -> Not sure if this is needed
	if (realname[0] != ':') {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
		return ;
	}
	realname = realname.substr(1);
	client.setUsername(username);
	client.setRealname(realname);
	// std::cout << "Debug print: " << client.getRealname() << std::endl;
	 std::cout << "Username [" << client.getUsername() << "] and realname [" << client.getRealname() << "]" << std::endl;
}
/**
 * @brief	handles the irc "NICK chosennick" command
 * @param	std::vector<std::string> divMsg whole command (NICK inluded as first element)
 * @param	Client &client who sent the NICK command
 * 
 */
void Handler::handleNickCmd(std::vector<std::string> divMsg , Client &client) {
	if (divMsg.size() == 1) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NONICKNAMEGIVEN_CODE + ERR_NONICKNAMEGIVEN + "\n", client.getSocketFd());
		return ;
	}
	if (!isNicknameValid(divMsg[1]))
	{
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_ERRONEUSNICKNAME_CODE + ERR_ERRONEUSNICKNAME + "\n", client.getSocketFd());
		return ;
	}
 	if (isNicknameInUse(divMsg[1], &client))
	{
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NICKNAMEINUSE_CODE + ERR_NICKNAMEINUSE + "\n", client.getSocketFd());
		return ;
	}
	client.setNickname(divMsg[1]);
	// Debug print
	std::cout << "Client nickname set to: " << divMsg[1] <<  std::endl;
}

void Handler::handlePingCmd(std::vector<std::string> input, Client &client) {
    std::string message = "PONG :" + input[1];
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

/*					*/
/*	 KICK command	*/
/*					*/

/*
void Handler::handleKickCmd(std::vector<std::string> input, Client &client)
{
	return;
}
*/
