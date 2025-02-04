#include "../inc/Handler.hpp"
#include "../inc/Server.hpp" 

#include <sstream>
#include <cctype>

std::list<Channel> 							Handler::channels;
std::map<std::string, modeHandler>			Handler::cmdModeMap;
std::map<std::string, modeHandlerNoArgv>	Handler::cmdModeMapNoArgv;

Handler::Handler(void) {
	initCmdMap();
	initModeCmdMaps();
}

void Handler::initCmdMap(void) {
	cmdMap["user"] = &handleUserCmd;
	cmdMap["nick"] = &handleNickCmd;
	cmdMap["ping"] = &handlePingCmd;
	cmdMap["join"] = &handleJoinCmd;
	cmdMap["topic"] = &handleTopicCmd;
	cmdMap["kick"] = &handleKickCmd;
	cmdMap["mode"] = &handleModeCmd;
	cmdMap["invite"] = &handleInviteCmd;
	cmdMap["privmsg"] = &handlePrivmsgCmd;
	cmdMap["quit"] = &handleQuitCmd;
	cmdMap["part"] = &handlePartCmd;
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

/**
 * @brief	directs each command to its respective function
 * @param	std::vector<std::string> input. Params passed, divided in a vector of strings
 * @param	Client	&client who sent the command
 */

void Handler::parseCommand(std::vector<std::string> input, Client &client) {

	std::string	command;
	if(input[0][0] == ':')
		command = input[1];
	else
		command = input[0];
	if (cmdMap.find(command) != cmdMap.end()) {
		cmdMap[command](input, client);
	} else
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_UNKNOWNCOMMAND_CODE + command + " " + ERR_UNKNOWNCOMMAND "\r\n", client.getSocketFd());
}

/**
 * @brief	handles the irc "USER <username> <hostname> <servername> :<realname>" command
 * @param	std::vector<std::string> input. Params passed divided in a vector of strings
 * @param	Client &client who sent the USER command
 */

void Handler::handleUserCmd(std::vector<std::string> input, Client &client) {
	if (input.size() < 4) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " USER " + ERR_NEEDMOREPARAMS "\r\n", client.getSocketFd());
		return ;
	}

	std::string	username = input[1];
	std::string hostname = input[2];
	std::string servername = input[3];
	std::string realname = "";

	int	vecSize	= input.size();
	for (int i = 4; i < vecSize; i++) {
		realname += input[i];
		if (i < vecSize - 1)
			realname += " ";
	}

	if (username.empty() || username.length() < 1 || username.length() > USERLEN) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " USER " + ERR_NEEDMOREPARAMS "\r\n", client.getSocketFd());
		return ;
	}

	if (hostname != "0") {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " USER " + ERR_NEEDMOREPARAMS "\r\n", client.getSocketFd());
		return ;
	}

	if (servername != "*") {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " USER " + ERR_NEEDMOREPARAMS "\r\n", client.getSocketFd());
		return ;
	}

	if (realname[0] != ':') {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " USER " + ERR_NEEDMOREPARAMS "\r\n", client.getSocketFd());
		return ;
	}
	realname = realname.substr(1);
	client.setUsername(username);
	client.setRealname(realname);
	client.setRegistered(true);
}

/**
 * @brief	handles the irc "NICK chosennick" command
 * @param	std::vector<std::string> input whole command (NICK inluded as first element)
 * @param	Client &client who sent the NICK command
 */

void Handler::handleNickCmd(std::vector<std::string> input , Client &client) {
	if (input.size() == 1) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NONICKNAMEGIVEN_CODE + ERR_NONICKNAMEGIVEN + "\r\n", client.getSocketFd());
		return ;
	}
	if (!isNicknameValid(input[1]))
	{
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_ERRONEUSNICKNAME_CODE + ERR_ERRONEUSNICKNAME + "\r\n", client.getSocketFd());
		return ;
	}
	if (isNicknameInUse(input[1], &client))
	{
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NICKNAMEINUSE_CODE + " * " + input[1] + " " + ERR_NICKNAMEINUSE + "\r\n", client.getSocketFd());
		return ;
	}
	sendResponse(":" + client.getNickname() + " NICK " + input[1] + "\r\n", client.getSocketFd());
	client.setNickname(input[1]);
}

/**
 * @brief	NICK auxiliary function to check whether the input nickname is valid or not according to RFC
 * @param	std::string chosen nickname to check its validity
 */

bool Handler::isNicknameValid(std::string nickname)
{
	// Check length (1 to 9 characters)
	if (nickname.empty() || nickname.length() > 9)
		return false;
	// Check the first character (no digits nor - from the allowed set of chars)
	char c = nickname[0];
	if (!(std::isalpha(c) || c == '_' || c == '[' || c == ']' || c == '\\' || c == '{' || c == '}' || c == '|'))
		return false;
	// Check the rest of the characters
	for (size_t i = 1; i < nickname.length(); ++i) {
		if (!(std::isalnum(nickname[i]) || nickname[i] == '_' || nickname[i] == '-' || nickname[i] == '[' || nickname[i] == ']' || nickname[i] == '\\' || nickname[i] == '{' || nickname[i] == '}' || nickname[i] == '|'))
			return false;
	}
	return true;
}

/**
 * @brief	NICK auxiliary function to check whether the input nickname is valid or not according to RFC
 * @param	std::string nickname to check if it's repeated
 * @param	Client	*client who sent the nickname, to be compared against server's existing nicknames
 */

bool Handler::isNicknameInUse(std::string nickname, Client *client)
{
	const Server* server = client->getServer();
	const std::list<Client>& clients = server->getClients();
	for (std::list<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->getNickname() == nickname) {
			return true;
		}
	}
	return false;
}

/**
 * @brief	handles the irc PRIVMSG command (sending a message to a user or a channel)
 * @param	std::vector<std::string> input whole command (message target -either a user or a channel- follows after "PRIVMSG" ); third parameter, message, must begin with :
 * @param	Client &client who sent the PRIVMSG command
 * 
 */
void Handler::handlePrivmsgCmd(std::vector<std::string> input , Client &client) {
	if(input[0][0] == ':')
		input.erase(input.begin());
	if (input.size() == 1 || input[1][0] == ':') {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NORECIPIENT_CODE + ERR_NORECIPIENT + "\r\n", client.getSocketFd());
		return ;
	}
	if (input.size() == 2) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOTEXTTOSEND_CODE + ERR_NOTEXTTOSEND + "\r\n", client.getSocketFd());
		return ;
	}
	if (input[2][0] != ':') {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_TOOMANYTARGETS_CODE + input[1] + input[2] + ERR_TOOMANYTARGETS + + "\r\n", client.getSocketFd());//should implement response for 3,4,5,.... targets
		return ;
	}
	if (input[1][0] != '#')
	{
		if(!isNicknameInUse(input[1], &client))
		{
			sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHNICK_CODE + " " + client.getNickname() + " " + input[1] + " " + ERR_NOSUCHNICK + "\r\n", client.getSocketFd());
			return ;
		}
		const Server* server = client.getServer();
		std::list<Client> clients = server->getClients();
		Client* foundClient = Client::findClientByName(input[1], clients);
		int targetFd = foundClient->getSocketFd();
		std::vector<std::string> subVector(input.begin() + 2, input.end());
		std::string outboundMessage = ":" + client.getNickname() + " PRIVMSG " + input[1] + " " + vectorToString(subVector, ' ') + "\r\n";
		sendResponse(outboundMessage, targetFd);
		return;
	}
	if (input[1][0] == '#')
	{
		std::list<Channel>::iterator itChannels = findChannel(input[1]);
		if (itChannels == channels.end())
		{
			sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHNICK_CODE + " " + client.getNickname() + " " + input[1] + " " + ERR_NOSUCHNICK + "\r\n", client.getSocketFd());
			return ;
		}
		std::vector<Client *> operators = itChannels->getOperators() ;
		std::vector<Client *> users = itChannels->getUsers() ;
		std::vector<Client *>::iterator itClients = operators.begin();
		std::vector<std::string> subVector(input.begin() + 2, input.end());
		while (itClients != operators.end())
		{
			if((*itClients)->getNickname() != client.getNickname())
			{
				std::string outboundMessage = ":" + client.getNickname() + " PRIVMSG " + input[1] + " " + vectorToString(subVector, ' ') + "\r\n";
				sendResponse(outboundMessage, (*itClients)->getSocketFd());
			}
			itClients++;
		}
		itClients = users.begin();
		while (itClients != users.end())
		{
			if((*itClients)->getNickname() != client.getNickname())
			{
				std::string outboundMessage = ":" + client.getNickname() + " PRIVMSG " + input[1] + " " + vectorToString(subVector, ' ') + "\r\n";
				sendResponse(outboundMessage, (*itClients)->getSocketFd());
			}
			itClients++;
		}
		return;
	}
}


/**
 * @brief	handles the irc QUIT command 
 * @param	std::vector<std::string> input whole command 
 * @param	Client &client who sent the QUIT command
 */

void Handler::handleQuitCmd(std::vector<std::string> input , Client &client) {
	if (input.size() >= 2 && input[1][0] == ':')
	{
		std::vector<std::string> subVector(input.begin() + 1, input.end());
		sendResponse("ERROR " + client.getNickname() + " (Quit " + vectorToString(subVector, ' ') + ")\r\n", client.getSocketFd());
	}
	else
		sendResponse("ERROR " + client.getNickname() + " (Quit)\r\n", client.getSocketFd());

	std::vector<Channel *> clientChannels = client.getClientChannels();
	std::vector<Channel *>::iterator itChannels = clientChannels.begin();
	while (itChannels != clientChannels.end())
	{
		(*itChannels)->removeClient(client.getNickname());
		std::vector<Client *> operators = (*itChannels)->getOperators() ;
		std::vector<Client *> users = (*itChannels)->getUsers() ;
		std::vector<Client *>::iterator itClients = operators.begin();
		while (itClients != operators.end())
		{
			sendResponse(":" + client.getNickname() + " QUIT :Client has left the server\r\n", (*itClients)->getSocketFd());
			itClients++;
		}
		itClients = users.begin();
		while (itClients != users.end())
		{
			sendResponse(":" + client.getNickname() + " QUIT :Client has left the server\r\n", (*itClients)->getSocketFd());
			itClients++;
		}
		itChannels++;
	}
	Server* server = const_cast<Server*>(client.getServer());
	server->disconnectClient(client.getSocketFd());
}

/**
 * @brief	Sends a pong to the client to check connection stability
 * @param	std::string server ip against which the ping has been sent
 * @param	Client &client who sent the ping command
 */

void Handler::handlePingCmd(std::vector<std::string> input, Client &client) {
	std::string message = "PONG " + input[1];
	sendResponse(message, client.getSocketFd());
}


/**
 * @brief	Sends a pong to the client to check connection stability
 * @param	std::string server ip against which the ping has been sent
 */

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
		std::cout << "Response sent to client: " << message << std::endl;
	}
}


void Handler::handleJoinCmd(std::vector<std::string> input, Client &client) {

	std::vector<std::string>				channelVector;
	std::vector<std::string>				passVector;
	std::map<std::string, std::string>		channelDictionary;

	if (input.size() <= 1)
	{
		Handler::sendResponse(Handler::prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " JOIN " + ERR_NEEDMOREPARAMS + "\r\n", client.getSocketFd());
		return;
	}

	std::vector<std::string>::iterator argvIt = input.begin() + 1;
	if (*argvIt == "0"){
		leaveAllChannels(client);
		return;
	}

	channelVector = getChannelVector(*argvIt, client);
	if (channelVector.empty())
		return;
	argvIt++;
	if (argvIt != input.end())
		passVector = getPassVector(*argvIt);
	channelDictionary = createDictionary(channelVector, passVector);
	joinCmdExec(channelDictionary, client);
}

std::vector<std::string> Handler::getChannelVector(std::string channelString, Client &client)
{
	std::vector<std::string>	channels;
	std::stringstream			ss(channelString);
	std::string					tempChannel;

	while(std::getline(ss, tempChannel, ','))
	{
		if (*tempChannel.begin() != '#')
			sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHCHANNEL_CODE + tempChannel + " " + ERR_NOSUCHCHANNEL + "\r\n", client.getSocketFd());
		else
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
		passwords.push_back(tempPass);
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
			authClientToChannel(*itChannels, itMap->second, client);
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

void Handler::authClientToChannel(Channel &channel, std::string &password, Client &client)
{
	bool passBool = true;
	bool limitBool = true;
	bool inviteBool = true;

	std::string nick = client.getNickname();
	if (channel.getClient(nick)){
		std::cerr << "JOIN ERROR: Client is already in the channel" << std::endl;
		return;
	}
	if (channel.getPassword() != "")
	{
		if (channel.getPassword() != password)
		{
			passBool = false;
			sendResponse(prependMyserverName(client.getSocketFd()) + ERR_BADCHANNELKEY_CODE + channel.getName() + " " + ERR_BADCHANNELKEY + "\r\n", client.getSocketFd());
		}
	}
	if (channel.getUserLimit() != 0)
	{
		if ((channel.getOperators().size() + channel.getUsers().size())  >= channel.getUserLimit())
		{
			limitBool = false;
			sendResponse(prependMyserverName(client.getSocketFd()) + ERR_CHANNELISFULL_CODE + channel.getName() + " " + ERR_CHANNELISFULL + "\r\n", client.getSocketFd());
		}
	}
	if (channel.getInviteMode() == true)
	{
		if (client.isInvited(channel))
			client.removeInvitation(channel);
		else
		{
			inviteBool = false;
			sendResponse(prependMyserverName(client.getSocketFd()) + ERR_INVITEONLYCHAN_CODE + channel.getName() + " " + ERR_INVITEONLYCHAN + "\r\n", client.getSocketFd());
		}
	}
	if (passBool == true && limitBool == true && inviteBool == true)
		addClientToChannel(channel, client);
}

void Handler::createChannel(std::string channelName, Client &client)
{
	Channel channel(client);

	channel.setName(channelName);
	channels.push_back(channel);
	client.addChannel(channels.back());
	sendMsgClientsInChannel(channel, client, "JOIN", "");
	sendResponse(prependMyserverName(client.getSocketFd()) + RPL_NAMREPLY_CODE + client.getNickname() + " = " + channelName + " :@" + client.getNickname() + "\r\n", client.getSocketFd());
	sendResponse(prependMyserverName(client.getSocketFd()) + RPL_ENDOFNAMES + client.getNickname() + " " + channelName + " :End of /NAMES list" + "\r\n", client.getSocketFd());
}

void Handler::addClientToChannel(Channel &channel, Client &client)
{
	channel.addUser(client);
	client.addChannel(channel);
	sendMsgClientsInChannel(channel, client, "JOIN", "");
	std::string clientsInChannel = getAllClientsInChannel(channel);
	sendResponse(prependMyserverName(client.getSocketFd()) + RPL_NAMREPLY_CODE + client.getNickname() + " = " + channel.getName() + " :" + clientsInChannel + "\r\n", client.getSocketFd());
	sendResponse(prependMyserverName(client.getSocketFd()) + RPL_ENDOFNAMES + client.getNickname() + " " + channel.getName() + " :End of /NAMES list" + "\r\n", client.getSocketFd());
	if (channel.getTopic() != "")
		sendResponse(prependMyserverName(client.getSocketFd()) + RPL_TOPIC_CODE + channel.getName() + " :" + channel.getTopic() + "\r\n", client.getSocketFd());
	return;
}

std::string Handler::getAllClientsInChannel(Channel &channel)
{
	std::string clientStr;
	std::vector<Client *> operators = channel.getOperators();
	std::vector<Client *> users = channel.getUsers();
	std::vector<Client *>::iterator itClients = operators.begin();
	while (itClients != operators.end())
	{
		clientStr.append("@" + (*itClients)->getNickname());
		if (itClients + 1 != operators.end())
			clientStr.append(" ");
		itClients++;
	}
	if (!users.empty() && !operators.empty())
		clientStr.append(" ");
	itClients = users.begin();
	while (itClients != users.end())
	{
		clientStr.append((*itClients)->getNickname());
		if (itClients + 1 != users.end())
			clientStr.append(" ");
		itClients++;
	}
	return (clientStr);
}

void Handler::handleTopicCmd(std::vector<std::string> input, Client &client)
{
	if (input.size() < 2)
	{
		Handler::sendResponse(Handler::prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " TOPIC " + ERR_NEEDMOREPARAMS + "\r\n", client.getSocketFd());
		return;
	}

	std::list<Channel>::iterator itChannel = findChannel(input[1]);
	if (itChannel == channels.end()){
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHCHANNEL_CODE + client.getNickname() + " " + input[1] + " " + ERR_NOSUCHCHANNEL + "\r\n", client.getSocketFd());
		return;
	}

	Channel *targetChannel = client.getChannel(input[1]);//RFC does not clarify if the client can get the topic of a channel he is not in. We assume he can´t
	if (!targetChannel){
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOTONCHANNEL_CODE + input[1] + " " + ERR_NOTONCHANNEL + "\r\n", client.getSocketFd());
		return;
	}
	if (input.size() == 2){
		if (targetChannel->getTopic() == ""){
			sendResponse(prependMyserverName(client.getSocketFd()) + RPL_NOTOPIC_CODE + targetChannel->getName() + " " + RPL_NOTOPIC + "\r\n", client.getSocketFd());
			return;
		}
		else{
			sendResponse(prependMyserverName(client.getSocketFd()) + RPL_TOPIC_CODE + targetChannel->getName() + " :" + targetChannel->getTopic() + "\r\n", client.getSocketFd());
			return;
		}
	}
	else
	{
		std::vector<std::string> vectorTopic(input.begin() + 2, input.end());
		std::string topic = vectorToString(vectorTopic, ' ');
		if (topic.at(0) == ':')
			topic.erase(topic.begin());
		if (targetChannel->getTopicMode() == true)
		{
			std::string userNick = client.getNickname();
			if (!targetChannel->getOperatorClient(userNick)){
				sendResponse(prependMyserverName(client.getSocketFd()) + ERR_CHANOPRIVSNEEDED_CODE + targetChannel->getName() + " " + ERR_CHANOPRIVSNEEDED + "\r\n", client.getSocketFd());
				return;
			}
		}
		targetChannel->setTopic(topic, client);
		std::cout << "topic: " << topic << std::endl;
		sendMsgClientsInChannel(*targetChannel, client, "TOPIC", topic);
		return;
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

void Handler::handleKickCmd(std::vector<std::string> input, Client &client)
{
	if (input.size() < 3)
	{
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + client.getNickname() + " KICK " + ERR_NEEDMOREPARAMS + "\r\n", client.getSocketFd());
		return;
	}
	std::list<Channel>::iterator itChannel = findChannel(input[1]);
	if (itChannel == channels.end()){
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHCHANNEL_CODE + client.getNickname() + " " + input[1] + " " + ERR_NOSUCHCHANNEL + "\r\n", client.getSocketFd());
		std::cerr << "KICK ERROR: Channel does not exist" << std::endl;
		return;
	}

	Channel *isInChannel = client.getChannel(input[1]);
	if (!isInChannel){
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOTONCHANNEL_CODE + client.getNickname() + " " + input[1] + " " + ERR_NOTONCHANNEL + "\r\n", client.getSocketFd());
		return;
	}

	if (itChannel->isClientOperator(client) == true)
	{
		std::vector<std::string> kickedClients = getPassVector(input[2]);
		std::vector<std::string>::iterator it = kickedClients.begin();
		while (it != kickedClients.end())
		{
			Client *clientPtr = itChannel->getClient(*it);
			if (!clientPtr){
				sendResponse(prependMyserverName(client.getSocketFd()) + ERR_USERNOTINCHANNEL_CODE + client.getNickname() + " " + *it + " " + input[1] + " " + ERR_USERNOTINCHANNEL + "\r\n", client.getSocketFd());
				std::cerr << "KICK ERROR: Target client is not in channel" << std::endl; //Try in hexchat
			}
			else{
				std::string msg = createKickMessage(input);
				sendMsgClientsInChannelKick(*isInChannel, client, "KICK", clientPtr->getNickname(), msg);
				clientPtr->removeChannel(input[1]);
				itChannel->removeClient(*it);
				if (itChannel->getUsers().empty() && itChannel->getOperators().empty())
					deleteChannel(channels, itChannel->getName());
			}
			it++;
		}
	}
	else
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_CHANOPRIVSNEEDED_CODE + client.getNickname() + " " + input[1] + " " + ERR_CHANOPRIVSNEEDED + "\r\n", client.getSocketFd());
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
	message.erase(0, 1);
	return message;
}

void Handler::handleModeCmd(std::vector<std::string> input, Client &client)
{
	(void)client;

	std::vector<std::string>	flagVector;
	std::vector<std::string>	argvVector;
	int							status;	

	status = 0;
	if (input.size() < 2){
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + client.getNickname() + " MODE " + ERR_NEEDMOREPARAMS + "\r\n", client.getSocketFd());
		return;
	}

	std::list<Channel>::iterator itChannel = findChannel(input[1]);
	if (itChannel == channels.end())
	{
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHCHANNEL_CODE + client.getNickname() + " " + input[1] + " " + ERR_NOSUCHCHANNEL + "\r\n", client.getSocketFd());
		return;
	}

	std::string temp = client.getNickname();
	if (!itChannel->getOperatorClient(temp)){
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_CHANOPRIVSNEEDED_CODE + client.getNickname() + " " + input[1] + " " + ERR_CHANOPRIVSNEEDED + "\r\n", client.getSocketFd());
		return;
	}

	if (input.size() == 2){
		sendChannelModeIs(client, (*itChannel));
		return;
	}

	if (parseFlagString(flagVector, input[2], client) == false){
		return;
	}

	std::vector<std::string>::iterator it = input.begin() + 3;
	while(it != input.end())
	{
		argvVector.push_back(*it);
		it++;
	}

	std::string flagSendStr;
	std::string argvSendStr;
	status = 0;
	int newStatus = 0;
	std::vector<std::string>::iterator flagIt = flagVector.begin();
	std::vector<std::string>::iterator argvIt = argvVector.begin();
	while (flagIt != flagVector.end())
	{
		if (cmdModeMapNoArgv.find(*flagIt) != cmdModeMapNoArgv.end()){
			if (cmdModeMapNoArgv[*flagIt](*itChannel) == true)
				appendToFlagStr(status, newStatus, *flagIt, flagSendStr);
		}
		else
		{
			if (argvIt != argvVector.end()){
				if (cmdModeMap[*flagIt](*itChannel, *argvIt) == true)
				{
					appendToFlagStr(status, newStatus, *flagIt, flagSendStr);
					argvSendStr.append(*argvIt);
					if (argvIt + 1 != argvVector.end())
						argvSendStr.append(" ");
				}
				else{
					if (*flagIt == "+k" || *flagIt == "-k"){
						if (itChannel->getPassword() != "")
							sendResponse(prependMyserverName(client.getSocketFd()) + ERR_KEYSET_CODE + client.getNickname() + " " + itChannel->getName() + " " + ERR_KEYSET + "\r\n", client.getSocketFd());
					}
				}
				argvIt++;
			}
			else
				sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + client.getNickname() + " MODE " + itChannel->getName() + " " + *flagIt + " " + ERR_NEEDMOREPARAMS + "\r\n", client.getSocketFd()); //This message is deprecated and hexchat does not use it, but it is useful for debug purposes
		}
		flagIt++;
	}
	if (flagSendStr.empty() == false)
		sendMsgClientsInChannelNoPrintCh(*itChannel, client, "MODE " + itChannel->getName() + " " + flagSendStr + " " + argvSendStr, "");
	return;
}

bool Handler::parseFlagString(std::vector<std::string> &flagVector, std::string flags, Client &client)
{
	int status = PLUS_STATUS;
	std::string ref = "itkol+-";

	std::string::iterator itStr = flags.begin();
	while (itStr != flags.end())
	{
		if (!isCharInStr(ref, *itStr)){
			sendResponse(prependMyserverName(client.getSocketFd()) + ERR_CHANOPRIVSNEEDED_CODE + client.getNickname() + " " + *itStr + " " + ERR_CHANOPRIVSNEEDED + "\r\n", client.getSocketFd());
			return (false);
		}
		if (*itStr == '+')
			status = PLUS_STATUS;
		else if(*itStr == '-')
			status = MINUS_STATUS;
		else{
			std::string addFlag;
			if (status == PLUS_STATUS)
				addFlag.append("+");
			else 
				addFlag.append("-");
			addFlag += *itStr;
			flagVector.push_back(addFlag);
		}
		itStr++;
	}
	return (true);
}

int		Handler::getStatusSymbol(std::string str)
{
	if (str[0] == '+')
		return (PLUS_STATUS);
	return (MINUS_STATUS);
}

void Handler::sendChannelModeIs(Client &client, Channel &channel)
{
	std::string flagStr("+");
	std::string argv;
	if (channel.getInviteMode() == true)
		flagStr.append("i");
	if (channel.getPassword() != ""){
		flagStr.append("k");
		argv.append(channel.getPassword());
	}
	if (channel.getUserLimit() != 0){
		flagStr.append("l");
		if (argv != "")
			argv.append(" ");
		std::stringstream ss;
		ss << channel.getUserLimit();
		argv.append(ss.str());
	}
	if (channel.getTopicMode() == true)
		flagStr.append("t");
	sendResponse(prependMyserverName(client.getSocketFd()) + RPL_CHANNELMODEIS_CODE + client.getNickname() + " " + flagStr + " " + argv + "\r\n", client.getSocketFd());
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

void Handler::appendToFlagStr(int &status, int &newStatus, std::string &flag, std::string &flagSendStr)
{
	newStatus = getStatusSymbol(flag);
	if (newStatus != status)
	{
		if (newStatus == PLUS_STATUS)
			flagSendStr.append("+");
		else
			flagSendStr.append("-");
	}
	status = newStatus;
	flagSendStr.append(1, flag[1]);
	return;
}

bool Handler::activateInviteMode(Channel &channel)
{
	if (channel.getInviteMode() == false)
	{
		channel.setInviteMode(true);
		return true;
	}
	return false;
}

bool Handler::deactivateInviteMode(Channel &channel)
{
	if (channel.getInviteMode() == true)
	{
		channel.setInviteMode(false);
		return true;
	}
	return false;
}

bool Handler::activateTopicPrivMode(Channel &channel)
{
	if (channel.getTopicMode() == false)
	{
		channel.setTopicMode(true);
		return true;
	}
	return false;
}

bool Handler::deactivateTopicPrivMode(Channel &channel)
{
	if (channel.getTopicMode() == true)
	{
		channel.setTopicMode(false);
		return true;
	}
	return false;
}

bool Handler::deactivateUserLimitMode(Channel &channel)
{
	if (channel.getUserLimit() != 0)
	{
		channel.setUserLimit(0);
		return true;
	}
	return false;
}

bool Handler::activateUserLimitMode(Channel &channel, std::string newLimit)
{
	unsigned int number;
	std::stringstream ss(newLimit);
	std::string::iterator it = newLimit.begin();

	if (*it == '+')
		it++;
	while(it != newLimit.end())
	{
		if (isdigit(*it) == false)
			return false;
		it++;
	}

	ss >> number;

	if (channel.getUserLimit() != number && number != 0)
	{
		channel.setUserLimit(number);
		return true;
	}
	return false;
}

bool Handler::activatePasswordMode(Channel &channel, std::string newPassword)
{
	if (channel.getPassword() == "")
	{
		channel.setPassword(newPassword);
		return true;
	}
	return false;
}

bool Handler::deactivatePasswordMode(Channel &channel, std::string newPassword)
{
	if (channel.getPassword() == newPassword && channel.getPassword() != "")
	{
		channel.setPassword("");
		return true;
	}
	return false;
}

bool Handler::activateOperatorMode(Channel &channel, std::string targetClient)
{
	Client* clientPtr = channel.getUserClient(targetClient);
	if (!clientPtr){
		return false;
	}
	channel.removeClient(targetClient);
	channel.addOperator(*clientPtr);
	std::cout << "Operator privileges granted to: " << targetClient << std::endl;
	return true;
}

bool Handler::deactivateOperatorMode(Channel &channel, std::string targetClient)
{
	Client* clientPtr = channel.getOperatorClient(targetClient);
	if (!clientPtr){
		std::cerr << "DEACTIVATE OPERATOR MODE: Target Client is not an operator" << std::endl;
		return false;
	}
	channel.removeClient(targetClient);
	channel.addUser(*clientPtr);
	std::cout << "Operator privileges removed to: " << targetClient << std::endl;
	return true;
}

/**
 * @brief	handles the irc "INVITE <username> <#channel>" command. Only operators can invite to channels in invite mode
 * @param	std::vector<std::string> input. Params passed divided in a vector of strings
 * @param	Client &client who sent the INVITE command
 */
void	Handler::handleInviteCmd(std::vector<std::string> input, Client &client) {
	if (input.size() < 3) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " INVITE " + ERR_NEEDMOREPARAMS "\r\n", client.getSocketFd());
		return ;
	}

	std::list<Channel>::iterator itChannel = findChannel(input[2]);
	if (itChannel == channels.end()){
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHCHANNEL_CODE + client.getNickname() + " " + input[1] + " " + ERR_NOSUCHCHANNEL + "\r\n", client.getSocketFd());
		return;
	}

	std::string	invitedNickname	= input[1];
	std::string	invChannelName	= input[2];
	Server				*server	= const_cast<Server *>(client.getServer());
	std::list<Client>	*clients	= server->getClientsPtr();//????
	Channel				*invitedChannel	= client.getChannel(invChannelName);
	if (!invitedChannel) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOTONCHANNEL_CODE + invChannelName + " " + ERR_NOTONCHANNEL + "\r\n", client.getSocketFd());
		return ;
	}

	if (invitedChannel->getInviteMode() && !invitedChannel->isClientOperator(client)) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_CHANOPRIVSNEEDED_CODE + invChannelName + " " + ERR_CHANOPRIVSNEEDED + "\r\n", client.getSocketFd());
		return ;
	}

	Client	*invitedClient = Client::findClientByName(invitedNickname, *clients);
	if (!invitedClient) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHNICK_CODE + invitedNickname + " " + ERR_NOSUCHNICK + "\r\n", client.getSocketFd());
		return ;
	}
	if (invitedClient->isClientInChannel(invChannelName)) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_USERONCHANNEL_CODE + invitedNickname + " " + invChannelName + " " + ERR_USERONCHANNEL + "\r\n", client.getSocketFd());
		return ;
	}
	if (invitedClient->isInvited(*invitedChannel) == false)
			invitedClient->addInvitedChannel(*invitedChannel);
	sendResponse(getClientPrefix(client) + " INVITE " + invitedNickname + " :" + invChannelName + "\r\n", invitedClient->getSocketFd());
	sendResponse(prependMyserverName(client.getSocketFd()) + RPL_INVITING_CODE + invChannelName + " " + invitedNickname + "\r\n", client.getSocketFd());
	return ;
}

std::string Handler::getClientPrefix(Client const &client) {
	std::string prefix(":");
	prefix.append(client.getNickname());
	prefix.append("!~");
	prefix.append(client.getUsername());
	prefix.append("@");
	prefix.append(client.getIpAddr());
	return (prefix);
}

void Handler::sendMsgClientsInChannel(Channel &channel, Client &client, std::string cmd, std::string argv){

	std::vector<Client *> operators = channel.getOperators();
	std::vector<Client *> users = channel.getUsers();
	std::vector<Client *>::iterator opIt = operators.begin();
	std::vector<Client *>::iterator usersIt = users.begin();

	std::string response;

	response.append(getClientPrefix(client));
	response.append(" ");
	response.append(cmd);
	response.append(" ");
	response.append(channel.getName());
	response.append(" ");
	if (argv != ""){
		if (argv[0] != ':')
			argv.insert(0, ":");
		response.append(argv);
	}
	response.append("\r\n");

	while (opIt != operators.end()){
		sendResponse(response, (*opIt)->getSocketFd());
		opIt++;
	}

	while (usersIt != users.end()){
		sendResponse(response, (*usersIt)->getSocketFd());
		usersIt++;
	}
	return;
}

void Handler::sendMsgClientsInChannelNoPrintCh(Channel &channel, Client &client, std::string cmd, std::string argv){

	std::vector<Client *> operators = channel.getOperators();
	std::vector<Client *> users = channel.getUsers();
	std::vector<Client *>::iterator opIt = operators.begin();
	std::vector<Client *>::iterator usersIt = users.begin();

	std::string response;

	response.append(getClientPrefix(client));
	response.append(" ");
	response.append(cmd);
	response.append(" ");
	if (argv != ""){
		if (argv[0] != ':')
			argv.insert(0, ":");
		response.append(argv);
	}
	response.append("\r\n");

	while (opIt != operators.end()){
		sendResponse(response, (*opIt)->getSocketFd());
		opIt++;
	}

	while (usersIt != users.end()){
		sendResponse(response, (*usersIt)->getSocketFd());
		usersIt++;
	}
	return;
}

void Handler::sendMsgClientsInChannelKick(Channel &channel, Client &client, std::string cmd, std::string kickedClient, std::string argv)
{
	std::vector<Client *> operators = channel.getOperators();
	std::vector<Client *> users = channel.getUsers();
	std::vector<Client *>::iterator opIt = operators.begin();
	std::vector<Client *>::iterator usersIt = users.begin();

	std::string response;

	response.append(getClientPrefix(client));
	response.append(" ");
	response.append(cmd);
	response.append(" ");
	response.append(channel.getName());
	response.append(" ");
	response.append(kickedClient);
	response.append(" ");
	if (argv != ""){
		if (argv[0] != ':')
			argv.insert(0, ":");
		response.append(argv);
	}
	response.append("\r\n");

	while (opIt != operators.end()){
		sendResponse(response, (*opIt)->getSocketFd());
		opIt++;
	}

	while (usersIt != users.end()){
		sendResponse(response, (*usersIt)->getSocketFd());
		usersIt++;
	}
	return;
}

void	Handler::handlePartCmd(std::vector<std::string> input, Client &client) {
	if (input.size() < 2) {
		sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + "PART " + ERR_NEEDMOREPARAMS + "\r\n", client.getSocketFd());
		return ;
	}

    std::vector<std::string> partChannels;
    std::istringstream stream(input[1]);
    std::string channelName;
    while (std::getline(stream, channelName, ',')) {
        partChannels.push_back(channelName);
    }

    std::string reason = "Leaving";
    if (input.size() > 2 && input[2][0] == ':') {
        std::vector<std::string> reasonParts(input.begin() + 2, input.end());
        reason = vectorToString(reasonParts, ' ');
    }

    for (size_t i = 0; i < partChannels.size(); ++i) {
        channelName = partChannels[i];
        Channel* channel = client.getChannel(channelName);
        if (!channel) {
            sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOSUCHCHANNEL_CODE + channelName + " " + ERR_NOSUCHCHANNEL + "\r\n", client.getSocketFd());
            continue;
        }

        if (!client.isClientInChannel(channelName)) {
            sendResponse(prependMyserverName(client.getSocketFd()) + ERR_NOTONCHANNEL_CODE + channelName + ERR_NOTONCHANNEL + "\r\n", client.getSocketFd());
            continue;
        }

        channel->removeClient(client.getNickname());
        client.removeChannel(channelName);
        
        sendResponse(getClientPrefix(client) + " PART " + channelName + " " + reason + "\r\n", client.getSocketFd());

		std::vector<Client*> users = channel->getUsers();
        for (std::vector<Client*>::iterator it = users.begin(); it != users.end(); ++it)
            sendResponse(getClientPrefix(client) + " PART " + channelName + " " + reason + "\r\n", (*it)->getSocketFd());
		std::vector<Client*> operators = channel->getOperators();
		for (std::vector<Client*>::iterator it = operators.begin(); it != operators.end(); ++it)
            sendResponse(getClientPrefix(client) + " PART " + channelName + " " + reason + "\r\n", (*it)->getSocketFd());

        if (channel->getUsers().empty() && channel->getOperators().empty())
            deleteChannel(channels, channelName);
    }
    return;
}

void	Handler::deleteChannel(std::list<Channel> &channels, std::string channelName) {
	if (channels.empty())
		return ;
	std::list<Channel>::iterator it	= channels.begin();
	while (it != channels.end()) {
		if (it->getName() == channelName){
			it = channels.erase(it);
			return;
		}
		it++;
	}
}

std::list<Channel>& Handler::getChannels() {
	return channels;
}

void	Handler::leaveAllChannels(Client &client)
{
	std::vector<Channel *> clientChannels = client.getClientChannels();
	std::vector<Channel *>::iterator it = clientChannels.begin();
	while(it != clientChannels.end())
	{
		std::vector<std::string> temp;
		temp.push_back("PART");
		temp.push_back((*it)->getName());
		handlePartCmd(temp, client);
		it++;
	}
}
