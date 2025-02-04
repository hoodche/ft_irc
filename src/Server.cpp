/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nvillalt <nvillalt@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/24 18:26:33 by igcastil          #+#    #+#             */
/*   Updated: 2025/02/04 16:42:04 by nvillalt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Handler.hpp"
#include <arpa/inet.h>
#include <stdexcept>
#include <poll.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <string.h>
#include <cctype>

Server::Server() : listenSocketFd(-1)
{
}
bool Server::signalReceived = false;

/**
 * @brief		triggered when SIGINT or SIGQUIT is received. Prints a message
 * 				and sets signalReceived to true (which is checked in the polling
 * 				 loop)
 * @param		int received signal
 */
void Server::signalHandler(int signal)
{
	if(signal == SIGINT)
		std::cout << std::endl << "Signal SIGINT Received!" << std::endl;
	else if(signal == SIGQUIT)
		std::cout << std::endl << "Signal SIGQUIT Received!" << std::endl;
	Server::signalReceived = true;
}

std::list<Client> Server::getClients(void) const
{
	return this->clients;
}

std::list<Client> *Server::getClientsPtr(void)
{
	return (&clients);
}
/**
 * @brief		closes all socket descriptors opened so far in the server 
 * 				(in the fds vector -listening socket + connected sockets)
 */
void	Server::closeFds()
{
	for (size_t i = 0; i < this->fds.size(); i++)
		close(this->fds[i].fd);
}

/**
 * @brief	opens the server listening socket (on every available network 
 * 			interface on the host machine and port specified in 2nd argument to 
 * 			program execution). Sets the socket to non-blocking (as required by 
 * 			project rules)
 */
void Server::initSocket()
{
	int socketOptionEnable = 1;
	this->serverAddress.sin_family = AF_INET;
	this->serverAddress.sin_port = htons(this->port);
	this->serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	this->listenSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenSocketFd == -1)
		throw(std::runtime_error("socket could not be created "));
	if(setsockopt(listenSocketFd, SOL_SOCKET, SO_REUSEADDR, &socketOptionEnable, sizeof(socketOptionEnable)) == -1)
		throw(std::runtime_error("could not set option SO_REUSEADDR to allow reuse of local addresses for socket"));
	if (fcntl(listenSocketFd, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("could not set option O_NONBLOCK for socket"));
	if (bind(listenSocketFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
		throw(std::runtime_error("could not bind server socket to its address and port"));
	if (listen(listenSocketFd, SOMAXCONN) == -1)
		throw(std::runtime_error("server can not listen on socket "));
	std::cout << "Server listening on port " << this->port << std::endl;
	this->connectedSocket.fd = listenSocketFd;
	this->connectedSocket.events = POLLIN;
	this->connectedSocket.revents = 0;
	this->fds.push_back(this->connectedSocket);
}

/**
 * @brief	runs server's main loop, which polls the fds vector for connection 
 * 			requests (on fds[0] server's listening socket) and irc commands (on 
 * 			rest of fds vector -"connected sockets"). Loop is exited when SIGINT
 * 			or SIGQUIT is received (signalReceived is set to true by 
 * 			signalHandler() function)
 * @param	int port (2nd arg to program execution)
 * @param	std::string password (3rd arg to program execution)
 */
void Server::init(int port, std::string password)
{
	this->password = password;
	this->port = port;
	this->initSocket();

	while (Server::signalReceived == false)
	{
		if(poll(&fds[0],fds.size(),-1) == -1  && Server::signalReceived == false)
			throw(std::runtime_error("server could not poll sockets fd's"));
		for (size_t i = 0; i < this->fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == listenSocketFd)
					this->acceptClient();
				else
					this->readFromFd(fds[i].fd);
			}
		}
	}
	this->closeFds();
}

/**
 * @brief	accepts a client's connection request (before any command is parsed)
 * 			and adds it and its socket fd to their corresponding vectors
 */
void Server::acceptClient()
{
	memset(&clientAddress, 0, sizeof(clientAddress));
	socklen_t clientAddressLength = sizeof(clientAddress);
	int connectedSocketFd = accept(this->listenSocketFd, (sockaddr *)&clientAddress, &clientAddressLength);
	if (connectedSocketFd == -1)
		{
			std::cout << "server could not accept incoming connection" << std::endl; 
			return;
		}
	if (fcntl(connectedSocketFd, F_SETFL, O_NONBLOCK) == -1)
		{
			std::cout << "fcntl() failed" << std::endl;
			return;
		}
	this->connectedSocket.fd = connectedSocketFd;
	this->connectedSocket.events = POLLIN;
	this->connectedSocket.revents = 0;
	this->fds.push_back(this->connectedSocket);
	clients.push_back(Client(connectedSocketFd, *this, std::string(inet_ntoa(clientAddress.sin_addr))));
	std::cout << "a new client from IP "<< inet_ntoa(clientAddress.sin_addr) << " and port " << ntohs(clientAddress.sin_port) << " has been connected with socket fd " << connectedSocketFd << std::endl;
}

/**
 * @brief	reads from a connected socket when poll function detects an event in
 * 			it (if client closes connection it is an event and 0 bytes are read,
 * 			hence server proceeds to close that connection)
 * @param	int clientConnectedfd socket fd to be read
 */
void Server::readFromFd(int clientConnectedfd)
{
	char	buffer[SOCKET_SIZE];
	ssize_t	bytesRead =recv(clientConnectedfd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead < 0)
		throw std::runtime_error("Server could not read incoming mesage ");
	else if (bytesRead == 0) {
		std::cout << "Client " << clientConnectedfd << " has closed the connection. calling disconnectClient()" << std::endl;
		disconnectClient(clientConnectedfd);
		return ;
	}
	buffer[bytesRead] = '\0';
	if (clientBuffers.find(clientConnectedfd) == clientBuffers.end())
		clientBuffers[clientConnectedfd] = "";
	clientBuffers[clientConnectedfd].append(buffer);

	size_t	pos;
	while (Client::findClientByFd(clientConnectedfd, clients) && (pos = clientBuffers[clientConnectedfd].find("\r\n")) != std::string::npos) {
		std::string message;
		if(pos >= 510)
			message = clientBuffers[clientConnectedfd].substr(0, 510);
		else
			message = clientBuffers[clientConnectedfd].substr(0, pos);
		clientBuffers[clientConnectedfd].erase(0, pos + 2);
		processMessage(clientConnectedfd, message);
	}
}

void toLowerCase(std::string& str) {
	for (size_t i = 0; i < str.size(); ++i) {
		str[i] = std::tolower(str[i]);
	}
}

/**
 * @brief	processes the received message and divides the string in a vector 
 * 			of strings. Message is a potential irc command, a substring read 
 * 			from the socket and delimited by \r\n 
 * @param	int fd socket fd where the message came from
 * @param	std::string received message (one of the potemtial irc commands read
 * 			in fd socket)
*/

void	Server::processMessage(int fd, std::string message) {
	std::cout << "potential irc command from fd "<< fd << ": " << message << std::endl;
	std::vector<std::string> divMsg = splitCmd(message);
	if (divMsg.empty())
		return;
	Client	*client = Client::findClientByFd(fd, clients);
	toLowerCase(divMsg[0]);
	if (client->isVerified() == false) {
		if (divMsg[0] == "cap") {
			if(divMsg.size() > 1) {
				toLowerCase(divMsg[1]); 
				if (divMsg[1] == "ls") {
					return ;
				}
			}
		}
		if (divMsg[0] == "pass") {
			if (divMsg.size() != 2) {
				Handler::sendResponse(Handler::prependMyserverName(client->getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + "PASS " + ERR_NEEDMOREPARAMS + "\r\n", fd);
				return ;
			}
			if (divMsg[1] == this->password) {
				client->setVerified(true);
				std::cout << "Client with fd " << fd << " password correct!" << std::endl;
			} else {
				Handler::sendResponse(Handler::prependMyserverName(client->getSocketFd()) + ERR_PASSWDMISMATCH_CODE + ERR_PASSWDMISMATCH + "\r\n", fd);
				return ;
			}
		}
	}
 	if (client->isRegistered() == false && client->isVerified()) {
		if (divMsg[0] == "nick")
			Handler::handleNickCmd(divMsg, *client);
		if (divMsg[0] == "user" && !client->getNickname().empty())
			Handler::handleUserCmd(divMsg, *client);
		if (!client->getUsername().empty() && !client->getNickname().empty())
			Handler::sendResponse(Handler::prependMyserverName(fd) + RPL_WELCOME_CODE + client->getNickname() + " " + ":Welcome to our IRC network, " + client->getNickname() + "\r\n", fd);
	} else if (client->isRegistered() && client->isVerified()) {
		if ((divMsg[0] == "user" || divMsg[0] == "pass") && client->isRegistered()) {
			Handler::sendResponse(Handler::prependMyserverName(fd) + ERR_ALREADYREGISTERED_CODE + ERR_ALREADYREGISTERED + "\r\n", fd);
			return ;
		}
		handler.parseCommand(divMsg, *client);
	}
} 

/**
 * @brief	splits the received message from client into strings to process in handle
 * @param	std::string received message trimmed without \r\n ending or any other trailing character
 */

std::vector<std::string>	Server::splitCmd(std::string trimmedMsg) {
	std::vector<std::string>	divMsg;
	std::istringstream			ss(trimmedMsg);
	std::string 				word;

	while (ss >> word)
		divMsg.push_back(word);

	return divMsg;
}

/**
 * @brief	prints every client held in server. Used for debug purposes
 */

void Server::printClients() const
{
	std::cout << "Currently connected clients and saved buffer:" << std::endl;

	std::map<int, std::string>::const_iterator it;
	for (it = clientBuffers.begin(); it != clientBuffers.end(); ++it) {
		int clientFd = it->first;  // The client socket FD
		const std::string& buffer = it->second;  // The client's accumulated buffer

		std::cout << "Client FD: " << clientFd << " - Buffer: [" << buffer << "]" << std::endl;
	}
}

/**
 * @brief	disconnects a client (closes its socket fd and deletes it and the 
 * 			client instance from their corresponding vectors). This happens when
 * 			client sent a wrong password or if client closed connection
 * @param	int clientConnectedfd socket fd to close
 */

void Server::disconnectClient(int clientConnectedfd)
{
	close(clientConnectedfd);
	std::list<Client>::iterator it = clients.begin();
	while(it != clients.end())
	{
		if (it->getSocketFd() == clientConnectedfd) {
			std::vector<Channel *> clientChannels = it->getClientChannels();
			std::vector<Channel *>::iterator itChannels = clientChannels.begin();
			while (itChannels != clientChannels.end())
			{
				(*itChannels)->removeClient(it->getNickname());
				if ((*itChannels)->getUsers().empty() && (*itChannels)->getOperators().empty())
					Handler::deleteChannel(Handler::getChannels(), (*itChannels)->getName());
				itChannels++;
			}
			clients.erase(it);
			break;
		}
		it++;
	}
	for (size_t i = 0; i < this->fds.size(); i++)
	{
		if (this->fds[i].fd == clientConnectedfd)
		{
			this->fds.erase(this->fds.begin() + i);
			break;
		}
	}
	clientBuffers.erase(clientConnectedfd);
	std::cout << "Client FD: " << clientConnectedfd << " has been disconnected" << std::endl;
}

/**
 * @brief	trims any starting or trailing whitespace \r or \n character
 * @param	std::string received message until \r\n is found
 */

std::string Server::trimMessage(std::string str) {
	  size_t first = str.find_first_not_of(" \t\r\n");
	  size_t last = str.find_last_not_of(" \t\r\n");
    if (first == std::string::npos || last == std::string::npos) {
        return "";
    } else {
        return str.substr(first, last - first + 1);
    }
}

