
#include "../inc/Client.hpp"

Client::Client(void): fd(-1), registered(false), oper(false), nick(""), username("") {}

Client::Client(int receivedFd): fd(receivedFd), registered(false), oper(false), nick(""), username("") {}

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

void Client::setNickname(std::string nickname) {
	nick = nickname;
	return ;
}

void Client::setUsername(std::string user) {
	username = user;
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