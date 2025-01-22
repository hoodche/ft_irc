#include "../inc/Handler.hpp"
#include "../inc/Server.hpp" 
#include "../commands/nick.cpp" 

#include <sstream>
#include <cctype>

std::list<Channel> Handler::channels; //Static variable must be declared outside the class so the linker can fint it. It is not vinculated to an object,
										//so the programmer have to do the job
std::map<std::string, modeHandler>		Handler::cmdModeMap;
std::map<std::string, modeHandlerNoArgv>	Handler::cmdModeMapNoArgv;

Handler::Handler(void) {
	initCmdMap(); // Initialise the command map
	initModeCmdMaps();
}

void Handler::initCmdMap(void) {
	cmdMap["user"] = &handleUserCmd;
	cmdMap["nick"] = &handleNickCmd;
	cmdMap["ping"] = &handlePingCmd;
	//cmdMap["pong"] = &handlePingCmd;
	cmdMap["join"] = &handleJoinCmd;
	cmdMap["topic"] = &handleTopicCmd;
	cmdMap["kick"] = &handleKickCmd;
	cmdMap["mode"] = &handleModeCmd;
	cmdMap["invite"] = &handleInviteCmd;
	cmdMap["privmsg"] = &handlePrivmsgCmd;
}

void Handler::initModeCmdMaps(void)
{
	cmdModeMap["+k"] = &activatePasswordMode;
	cmdModeMap["-k"] = &deactivatePasswordMode;
	cmdModeMap["+o"] = &activateOperatorMode;
	cmdModeMap["-o"] = &deactivateOperatorMode;
	cmdModeMap["+l"] = &activateUserLimitMode;

	cmdModeMapNoArgv["-l"] = &deactivateUserLimitMode;
	cmdModeMapNoArgv["+i"] = &activateInviteMode;
	cmdModeMapNoArgv["-i"] = &deactivateInviteMode;
	cmdModeMapNoArgv["+t"] = &activateTopicPrivMode;
	cmdModeMapNoArgv["-t"] = &deactivateTopicPrivMode;
}

void Handler::parseCommand(std::vector<std::string> divMsg, Client &client) {
	// Check if the command exists in the map. Command extracted as first member of str vector
	// If command exists and all is good, delete command from str vector
	// Esto solo está puesto para evitar el -Werror de momento
	std::string	command;
	if(divMsg[0][0] == ':')//first parameter is sender´s nickname
		command = divMsg[1];
	else
		command = divMsg[0];
	if (cmdMap.find(command) != cmdMap.end()) {
		cmdMap[command](divMsg, client);
	} else {
		std::cout << "could not find " << command << " in our internal server commands map" << std::endl;
	}
}

/**
 * @brief	handles the irc "USER <username> <hostname> <servername> :<realname>" command
 * @param	std::vector<std::string> divMsg. Params passed divided in a vector of strings
 * @param	Client &client who sent the USER command
 */

void Handler::handleUserCmd(std::vector<std::string> divMsg, Client &client) {
	// Generally seen like this: <username> 0 * <realname> as per this documentation - https://modern.ircdocs.horse/#user-message
	// Default client received command: USER nerea 0 * :realname -> input = nerea 0 * :realname

	// Check that we have the 4 required parameters	
	client.setRegistered(true);
	if (divMsg.size() < 4) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " USER " + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
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
	
	// Check that username goes according to what is expected, otherwise send an error to the client
	if (username.empty() || username.length() < 1 || username.length() > USERLEN) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " USER " + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
		return ;
	}

	// Check that the hostname is as expected
	if (hostname != "0") {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " USER " + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
		return ;
	}

	// Check that the servername is as expected
	if (servername != "*") {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " USER " + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
		return ;
	}

	// Check that realname starts with a : -> Not sure if this is needed
	if (realname[0] != ':') {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " USER " + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
		return ;
	}
	realname = realname.substr(1);
	client.setUsername(username);
	client.setRealname(realname);
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
	sendResponse(":" + client.getNickname() + " NICK " + divMsg[1] + "\n", client.getSocketFd());
	client.setNickname(divMsg[1]);
}

/**
 * @brief	handles the irc PRIVMSG command (sending a message to a user or a channel)
 * @param	std::vector<std::string> divMsg whole command (message target -either a user or a channel- follows after "PRIVMSG" )
 * @param	Client &client who sent the PRIVMSG command
 * 
 */
void Handler::handlePrivmsgCmd(std::vector<std::string> divMsg , Client &client) {
	if(divMsg[0][0] == ':')//first parameter is sender´s nickname
		divMsg.erase(divMsg.begin());//we do not need sender's nickname, since we have the client object as 2nd argument
	if (divMsg.size() == 1 || divMsg[1][0] == ':'){
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NORECIPIENT_CODE + ERR_NORECIPIENT + "\n", client.getSocketFd());
		return ;
	}
	if (divMsg.size() == 2){
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOTEXTTOSEND_CODE + ERR_NOTEXTTOSEND + "\n", client.getSocketFd());
		return ;
	}
	if (divMsg[2][0] != ':'){
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_TOOMANYTARGETS_CODE + divMsg[1] + divMsg[2] + ERR_TOOMANYTARGETS + "\n", client.getSocketFd());//should implement response for 3,4,5,.... targets
		return ;
	}
	if (divMsg[1][0] != '#')// message target is a user
	{
		if(!isNicknameInUse(divMsg[1], &client))
		{
			sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHNICK_CODE + " " + client.getNickname() + " " + divMsg[1] + " " + ERR_NOSUCHNICK, client.getSocketFd());
			return ;
		}
		//send message to divMsg[1] user
		const Server* server = client.getServer();
		std::list<Client> clients = server->getClients();
		Client* foundClient = Client::findClientByName(divMsg[1], clients);
		int targetFd = foundClient->getSocketFd();
		std::vector<std::string> subVector(divMsg.begin() + 2, divMsg.end());
		std::string outboundMessage = ":" + client.getNickname() + " PRIVMSG " + divMsg[1] + " " + vectorToString(subVector, ' ') + "\n";
		sendResponse(outboundMessage, targetFd);
		return;
	}
	if (divMsg[1][0] == '#')// message target is a channel
	{
		std::list<Channel>::iterator itChannels = findChannel(divMsg[1]);
		if (itChannels == channels.end())
		{
			sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHNICK_CODE + " " + client.getNickname() + " " + divMsg[1] + " " + ERR_NOSUCHNICK, client.getSocketFd());
			return ;
		}
		//send message to itChannels channel
		std::vector<Client *> operators = itChannels->getOperators() ;
		std::vector<Client *> users = itChannels->getUsers() ;
		std::vector<Client *>::iterator itClients = operators.begin();
		std::vector<std::string> subVector(divMsg.begin() + 2, divMsg.end());
		while (itClients != operators.end())
		{
			std::string outboundMessage = ":" + client.getNickname() + " PRIVMSG " + divMsg[1] + " " + vectorToString(subVector, ' ') + "\n";
			sendResponse(outboundMessage, (*itClients)->getSocketFd());
			itClients++;
		}
		itClients = users.begin();
		while (itClients != users.end())
		{
			std::string outboundMessage = ":" + client.getNickname() + " PRIVMSG " + divMsg[1] + " " + vectorToString(subVector, ' ') + "\n";
			sendResponse(outboundMessage, (*itClients)->getSocketFd());
			itClients++;
		}
		return;
	}
}

void Handler::handlePingCmd(std::vector<std::string> input, Client &client) {
	std::string message = "PONG " + input[1];
	sendResponse(message, client.getSocketFd());
}

/* void Handler::handlePongCmd(std::vector<std::string> input, Client &client) {//weird behavior, why server is sending a PONG command  after receiving clients PONG command (generated as acknowledge to server´s PING)?
	//server does nothing when receives a PONG command from a client
	(void)input;//needed to avoid compilation warning
	(void)client;
} */
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

void Handler::handleKickCmd(std::vector<std::string> input, Client &client)
{
	if (input.size() < 3)
	{
		std::cerr << "KICK ERROR: Incorrect format" << std::endl;
		return;
	}
	try{
		std::list<Channel>::iterator itChannel = findChannel(input[1]);
		if (itChannel == channels.end())
			throw std::out_of_range("KICK ERROR: Channel does not exists");
		if (itChannel->isClientOperator(client))
		{
			Client *clientPtr = itChannel->getClient(input[2]);
			clientPtr->removeChannel(input[1]);
			itChannel->removeClient(input[2]);
			std::cout << createKickMessage(input) << std::endl; //Created msg we should send to client
		}
	}catch(std::exception &e){
		std::cerr << e.what() << std::endl;
	}
	return;
}

std::string Handler::createKickMessage(std::vector<std::string> &input)
{
	std::string message = "";

	if (input.size() >= 4)
	{
		std::vector<std::string> subVector(input.begin() + 3, input.end());
		message = vectorToString(subVector, ' ');
		if (message.at(0) != ':')
		{
			std::cerr << "KICK ERROR: Incorrect format" << std::endl;
			return ("");
		}
	}
	return message;
}

/*					*/
/*	 MODE command	*/
/*					*/

void Handler::handleModeCmd(std::vector<std::string> input, Client &client)
{
	(void)client;

	std::vector<std::string>	flagVector;
	std::vector<std::string>	argvVector;
	int							status;	

	status = 0;
	if (input.size() < 3){
		std::cerr << "not enough argv" << std::endl;
		return;
	}

	std::list<Channel>::iterator itChannel = findChannel(input[1]);
	if (itChannel == channels.end())
	{
		std::cerr << "Error: Channel does not exists" << std::endl;
		return;
	}
	try{
		std::string temp = client.getNickname();
		itChannel->getOperatorClient(temp);
	}catch(std::exception &e){
		std::cout << "Error: client does not have operator privileges" << std::endl;
		return;
	}

	std::vector<std::string>::iterator it = input.begin() + 2;
	while(it != input.end())
	{
		parseModeString(flagVector, argvVector, status, *it);
		it++;
	}
	/*
	std::cout << "flagVector: ";
	it = flagVector.begin();
	while (it != flagVector.end())
	{
		std::cout << *it << " ";
		it++;
	}
	std::cout << std::endl;
	std::cout << "argvVector: ";
	it = argvVector.begin();
	while (it != argvVector.end())
	{
		std::cout << *it << " ";
		it++;
	}
	std::cout << std::endl;
	*/
	std::vector<std::string>::iterator flagIt = flagVector.begin();
	std::vector<std::string>::iterator argvIt = argvVector.begin();
	while (flagIt != flagVector.end())
	{
		if (cmdModeMapNoArgv.find(*flagIt) != cmdModeMapNoArgv.end())
			cmdModeMapNoArgv[*flagIt](*itChannel);
		else
		{
			if (argvIt != argvVector.end())
			{
				cmdModeMap[*flagIt](*itChannel, *argvIt);
				argvIt++;
			}
		}
		flagIt++;
	}
	return;
}

void Handler::parseModeString(std::vector<std::string> &flagVector, std::vector<std::string> &argvVector, int &status, std::string const &modeStr)
{
	std::string::const_iterator itStr = modeStr.begin();
	std::string ref= "itkol";
	bool		isFlag = false;

	getStatus(*itStr, status);
	while(itStr != modeStr.end())
	{
		if (status == ARGV_STATUS)
		{
			argvVector.push_back(modeStr);
			return;
		}
		if (isCharInStr(ref, *itStr))
		{
			addModeFlag(flagVector, status, *itStr);
			isFlag = true;
		}
		else
			getStatus(*itStr, status);
		itStr++;
	}
	if (isFlag == false)
		argvVector.push_back(modeStr);
	return;
}

void Handler::getStatus(const char &symbol, int &status)
{
	if (symbol == '+' && status != ARGV_STATUS)
		status = PLUS_STATUS;
	else if (symbol == '-' && status != ARGV_STATUS)
		status = MINUS_STATUS;
	else
		status = ARGV_STATUS;
	return;
}

bool Handler::isCharInStr(std::string const &ref, const char &c)
{
	std::string::const_iterator itStr = ref.begin();
	while(itStr != ref.end()){
		if (*itStr == c)
			return (true);
		itStr++;
	}
	return (false);
}

void Handler::addModeFlag(std::vector<std::string> &flagVector, int &status, char c)
{
	if (status == PLUS_STATUS)
	{
		std::string flagWithPlus = std::string("+")  + c;
		flagVector.push_back(flagWithPlus);
	}
	else
	{
		std::string flagWithMinus = std::string("-")  + c;
		flagVector.push_back(flagWithMinus);
	}
}


//Mode Function Pointers
void Handler::activateInviteMode(Channel &channel)
{
	if (channel.getInviteMode() == false)
	{
		channel.setInviteMode(true);
		std::cout << "Invite mode restrictions activated" << std::endl;
	}
	return;
}

void Handler::deactivateInviteMode(Channel &channel)
{
	if (channel.getInviteMode() == true)
	{
		channel.setInviteMode(false);
		std::cout << "Invite mode restrictions deactivated" << std::endl;
	}
	return;
}

void Handler::activateTopicPrivMode(Channel &channel)
{
	if (channel.getTopicMode() == false)
	{
		channel.setTopicMode(true);
		std::cout << "Topic mode restrictions activated" << std::endl;
	}
	return;
}

void Handler::deactivateTopicPrivMode(Channel &channel)
{
	if (channel.getTopicMode() == true)
	{
		channel.setTopicMode(false);
		std::cout << "Topic mode restrictions deactivated" << std::endl;
	}
	return;
}

void Handler::deactivateUserLimitMode(Channel &channel)
{
	if (channel.getUserLimit() != 0)
	{
		channel.setUserLimit(0);
		std::cout << "Limit mode restrictions deactivated" << std::endl;
	}
}

void Handler::activateUserLimitMode(Channel &channel, std::string newLimit)
{
	unsigned int number;
	std::stringstream ss(newLimit);
	std::string::iterator it = newLimit.begin();

	while(it != newLimit.end())
	{
		if (isdigit(*it) == false)
			return;
		it++;
	}

	ss >> number;

	if (channel.getUserLimit() == 0)
	{
		channel.setUserLimit(number);
		std::cout << "Limit mode restrictions activated: " << number << std::endl;
	}
}

void Handler::activatePasswordMode(Channel &channel, std::string newPassword)
{
	if (channel.getPassword() == "")
	{
		channel.setPassword(newPassword);
		std::cout << "Password mode restrictions activated" << std::endl;
	}
}

void Handler::deactivatePasswordMode(Channel &channel, std::string newPassword)
{
	if (channel.getPassword() == newPassword && channel.getPassword() != "")
	{
		channel.setPassword("");
		std::cout << "Password mode restrictions deactivated" << std::endl;
	}
}

void Handler::activateOperatorMode(Channel &channel, std::string targetClient)
{
	try{
		Client* clientPtr = channel.getUserClient(targetClient);
		channel.removeClient(targetClient);
		channel.addOperator(*clientPtr);
		std::cout << "Operator privileges granted to: " << targetClient << std::endl;
	}catch(std::exception &e){}
}

void Handler::deactivateOperatorMode(Channel &channel, std::string targetClient)
{
	try{
		Client* clientPtr = channel.getOperatorClient(targetClient);
		channel.removeClient(targetClient);
		channel.addUser(*clientPtr);
		std::cout << "Operator privileges removed to: " << targetClient << std::endl;
	}catch(std::exception &e){}
}

/*					*/
/*	 INVITE command	*/
/*					*/

/**
 * @brief	handles the irc "INVITE <username> <#channel>" command. Only operators can invite to channels in invite mode
 * @param	std::vector<std::string> input. Params passed divided in a vector of strings
 * @param	Client &client who sent the INVITE command
 */

// As usual, depending on RFC definition, some points might differ. Following Modern IRC definition here:
// https://modern.ircdocs.horse/#invite-message

void	Handler::handleInviteCmd(std::vector<std::string> input, Client &client) {
	if (input.size() < 3) {
		std::cout << "INVITE cmd needs three arguments INVITE <nickname> <channel>" << std::endl,
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " INVITE " + ERR_NEEDMOREPARAMS "\n", client.getSocketFd());
		return ;
	}

	std::string	invitedNickname	= input[1];
	std::string	invChannelName	= input[2];

	// Find the appropiate channel in our channels list
	Channel				*invitedChannel	= client.getChannel(invChannelName);
	const Server		*server	= client.getServer();
	std::list<Client>	clients	= server->getClients();
	if (!invitedChannel) {
		std::cout << invChannelName << " does not exist" << std::endl;
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHCHANNEL_CODE + " " + client.getNickname() + " " + invChannelName + " " + ERR_NOSUCHCHANNEL + "\n", client.getSocketFd());
		return ;
	}

	// Check that client IS an operator
	if (!invitedChannel->isClientOperator(client)) {
		std::cout << "Client " << client.getNickname() << " is not an operator of channel " << invChannelName << std::endl;
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_CHANOPRIVSNEEDED_CODE + " " + client.getNickname() + " " + invChannelName + " " + ERR_CHANOPRIVSNEEDED + "\n", client.getSocketFd());
		return ;
	}

	// Check that channel is in invite mode
	if (!invitedChannel->getMode("i")) {
		std::cout << "Channel " << invChannelName << " does not allow invites" << std::endl;
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_CHANOPRIVSNEEDED_CODE + " " + client.getNickname() + " " + invChannelName + " " + ERR_CHANOPRIVSNEEDED + "\n", client.getSocketFd());
		return ;
	}

	// Check if the client is Already in the channel
	Client	*invitedClient = Client::findClientByName(invitedNickname, clients);
	if (!invitedClient) {
		std::cout << "Invited client does not exist" << std::endl;
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHNICK_CODE + " " + client.getNickname() + " " + invitedNickname + " " + ERR_NOSUCHNICK, client.getSocketFd());
		return ;
	}
	std::string	channelsName = invitedChannel->getName();
	if (invitedClient->isClientInChannel(channelsName)) {
		std::cout << "Invited client is already in the invited channel" << std::endl;
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_USERONCHANNEL_CODE + " " + client.getNickname() + " " + invitedNickname + " " + invChannelName + " " + ERR_USERONCHANNEL, client.getSocketFd());
		return ;
	}
	// Inform the invited client that they have been invited
	sendResponse(prependMyserverName(client.getSocketFd()) + " INVITE " + invitedNickname + " " + invChannelName + "\n", invitedClient->getSocketFd());
	// Inform user that invite was successfully issued
	sendResponse(prependMyserverName(client.getSocketFd()) + invitedNickname + " has been invited to " + invChannelName + "\n", client.getSocketFd());
	return ;
}
