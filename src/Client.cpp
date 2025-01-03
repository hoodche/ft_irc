
#include "../inc/Client.hpp"

Client::Client(void): fd(-1), registered(false) {}

Client::Client(int receivedFd): fd(receivedFd), registered(false) {}

Client::~Client(void) {}

int Client::getSocketFd(void) const {
	return fd;
}