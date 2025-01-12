
#include "../inc/Client.hpp"
#include "../inc/Channel.hpp"

Client::Client(void): 
	fd(-1), verified(false), registered(false),
	//oper(false),
	 nick(""), username("") {}

Client::Client(int receivedFd):
	fd(receivedFd), verified(false) ,registered(false),
	//oper(false),
	 nick(""), username("") {}

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

std::string	Client::getRealname(void) const {
	return this->realname;
}

bool Client::isVerified(void) const {
	return this->verified;
}

bool Client::isRegistered(void) const {
	return this->registered;
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

Client* Client::findClientByFd(int fd, std::vector<Client> &clients) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getSocketFd() == fd) {
            return &clients[i];
        }
    }
    return NULL;
}

void Client::addClientChannel(Channel &channel)
{
	clientChannels.push_back(&channel);
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
