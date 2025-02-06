#include "../inc/Client.hpp"
#include "../inc/Channel.hpp"

Client::Client(int receivedFd, const Server &newServer, std::string ip):
	fd(receivedFd), verified(false) ,registered(false),
	 nick(""), username(""), ipAddr(ip), connectedToServer(&newServer) {}

Client::~Client(void) {}

int Client::getSocketFd(void) const {
	return this->fd;
}

std::string Client::getNickname(void) const {
	return this->nick;
}

std::string Client::getUsername(void) const {
	return this->username;
}

const Server* Client::getServer(void) const {
	return this->connectedToServer;
}

std::string	Client::getRealname(void) const {
	return this->realname;
}

std::vector<Channel *> &Client::getClientChannels(void) {
	return this->clientChannels;
}

bool Client::isVerified(void) const {
	return this->verified;
}

bool Client::isRegistered(void) const {
	return this->registered;
}

bool Client::isInvited(Channel &channel) const{
	std::vector<Channel *>::const_iterator it = invitedChannels.begin();
	while (it != invitedChannels.end())
	{
		if ((*it)->getName() == channel.getName())
			return (true);
		it++;
	}
	return (false);
}

void Client::removeInvitation(Channel &channel){
	std::vector<Channel *>::iterator it = invitedChannels.begin();
	while (it != invitedChannels.end())
	{
		if ((*it)->getName() == channel.getName())
		{
			invitedChannels.erase(it);
			return;
		}
		it++;
	}
	return;
}
void Client::setNickname(std::string nickname) {
	this->nick = nickname;
	return ;
}

void Client::setUsername(std::string user) {
	this->username = user;
	return ;
}

void Client::setRealname(std::string realname) {
	this->realname = realname;
	return ;
}

void Client::setVerified(bool tf) {
	this->verified = tf;
	return ;
}

void Client::setRegistered(bool tf) {
	this->registered = tf;
	return ;
}

Client* Client::findClientByFd(int fd, std::list<Client> &clients) {
	std::list<Client>::iterator it = clients.begin();
    while(it != clients.end()) {
        if (it->getSocketFd() == fd) {
            return &(*it);
        }
		it++;
    }
    return NULL;
}

Client* Client::findClientByName(std::string name, std::list<Client> &clients) {
	std::list<Client>::iterator it = clients.begin();
    while(it != clients.end()) {
        if (it->getNickname() == name) {
            return &(*it);
        }
		it++;
    }
    return NULL;
}

void	Client::addInvitedChannel(Channel &invitedChannel) {
	invitedChannels.push_back(&invitedChannel);
	return ;
}

bool Client::isClientInChannel(std::string &channelName)
{
	std::vector<Channel*>::iterator it = clientChannels.begin();
	if (!clientChannels.empty()) {
		while (it != clientChannels.end()) {
			if ((*it)->getName() == channelName) {
				return true;
			}
			it++;
		}
	}
	return false;
}

void Client::addChannel(Channel &newChannel)
{
	clientChannels.push_back(&newChannel);
	return ;
}

void	Client::removeInvitedChannels(std::string &channelName) {
	if (invitedChannels.empty())
		return;

	std::vector<Channel *>::iterator it = invitedChannels.begin();
	while (it != invitedChannels.end())
	{
		if ((*it)->getName() == channelName)
		{
			invitedChannels.erase(it);
			return ;
		}
		it++;
	}
}

Channel	*Client::getChannel(std::string &channelStr)
{
	std::vector<Channel*>::iterator itChannels = clientChannels.begin();

	if (!clientChannels.empty())
	{
		while (itChannels != clientChannels.end())
		{
			if ((*itChannels)->getName() == channelStr)
				return (*itChannels);
			itChannels++;
		}
	}
	return NULL;
}

void	Client::removeChannel(std::string &channelStr)
{
	std::vector<Channel*>::iterator itChannels = clientChannels.begin();

	if (clientChannels.empty())
		return;
	while (itChannels != clientChannels.end())
	{
		if ((*itChannels)->getName() == channelStr)
		{
			clientChannels.erase(itChannels);
			return;
		}
		itChannels++;
	}
}

std::string	Client::getIpAddr(void) const{
	return ipAddr;
}
