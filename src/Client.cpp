
#include "../inc/Client.hpp"
#include "../inc/Channel.hpp"

/*
Client::Client(void): 
	fd(-1), verified(false), registered(false),
	//oper(false),
	 nick(""), username("") {}
*/

Client::Client(int receivedFd, const Server &newServer):
	fd(receivedFd), verified(false) ,registered(false),
	//oper(false),
	 nick(""), username(""), server(&newServer) {}

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

bool Client::isVerified(void) const {
	return this->verified;
}

bool Client::isRegistered(void) const {
	return this->registered;
}

void Client::setNickname(std::string nickname) {
	nick = nickname;
	return ;
}

void Client::setUsername(std::string user) {
	username = user;
	return ;
}

void Client::setVerified(bool tf) {
	verified = tf;
	return ;
}

void Client::setRegistered(bool tf) {
	registered = tf;
	return ;
}

Client* Client::findClientByFd(int fd, std::list<Client> &clients) {
	std::list<Client>::iterator it = clients.begin();
    while(it != clients.end()) {
        if (it->getSocketFd() == fd) {
            return &(*it);
        }
    }
    return NULL;
}

bool Client::isClientInChannel(std::string channelName)
{
	std::vector<Channel *>::iterator it = clientChannels.begin();
	while(!clientChannels.empty() && (*it)->getName() != channelName && it != clientChannels.end())
		it++;
	if (it == clientChannels.end())
		return false;
	else
		return true;
}

Channel	*Client::getChannel(std::string &channelStr)
{
	std::vector<Channel*>::iterator itChannels = clientChannels.begin();
	while (!clientChannels.empty() && itChannels != clientChannels.end())
	{
		if ((*itChannels)->getName() == channelStr)
			return (*itChannels);
		itChannels++;
	}
	throw std::out_of_range("Invalid Channel");
}

void Client::addChannel(Channel &newChannel)
{
	clientChannels.push_back(&newChannel);
}
