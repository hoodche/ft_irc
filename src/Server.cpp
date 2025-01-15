/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nvillalt <nvillalt@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/24 18:26:33 by igcastil          #+#    #+#             */
/*   Updated: 2025/01/15 19:48:38 by nvillalt         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Handler.hpp"
#include <arpa/inet.h> // for htons
#include <stdexcept>   // for std::runtime_error
#include <poll.h>      // for POLLIN
#include <fcntl.h>     // for fcntl
#include <iostream>
#include <unistd.h> // for read
#include <csignal> //for SIGINT and SIGQUIT
#include <string.h> //for memset
#include <cctype>  // for std::tolower

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

std::vector<Client> Server::getClients(void) const
{
	return this->clients;
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
	this->serverAddress.sin_family = AF_INET; //address family for IPv4 addresses
	this->serverAddress.sin_port = htons(this->port);//port number. The htons() stands for "Host TO Network Short." It is used to convert a 16-bit number
													//from host byte order (endianness) to network byte order. In most systems, the host byte order is little-endian,
													//however, network protocols like TCP/IP use big-endian order.
	this->serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); //This macro allows the socket to bind to all available interfaces on the host machine. It is typically used in server applications to accept connections from any IP address. (htonl() is here for the sake of clarity since it is redundant due to macro INADDR_ANY expansion to 0)
	this->listenSocketFd = socket(AF_INET, SOCK_STREAM, 0);	//SOCK_STREAM indicates that the socket type is stream-oriented, which means it provides a reliable, two-way, connection-based byte stream. This type of socket is typically used for TCP connections. TCP is a protocol that ensures data is delivered in the same order it was sent and without errors, making it suitable for applications that require reliable communication, such as web servers, email clients, and file transfer programs.
														//3rd arg is the protocol to be used with the socket. Typically, this is set to 0 to use the default protocol for the specified address family (1st arg) and socket type (2nd arg)
	if(listenSocketFd == -1)
		throw(std::runtime_error("socket could not be created "));
	if(setsockopt(listenSocketFd, SOL_SOCKET, SO_REUSEADDR, &socketOptionEnable, sizeof(socketOptionEnable)) == -1)//allows to customize the behavior of a socket. 2nd arg specifies the protocol level at which the option resides (common values include SOL_SOCKET for socket-level options and IPPROTO_TCP for TCP-level options). 3rd arg specifies the option to be set (SO_REUSEADDR to allow reuse of local addresses)
		throw(std::runtime_error("could not set option SO_REUSEADDR to allow reuse of local addresses for socket"));
	if (fcntl(listenSocketFd, F_SETFL, O_NONBLOCK) == -1)//The fcntl function is used to perform various operations on file descriptors, including sockets. One common use of fcntl is to set a socket to non-blocking mode, which allows the program to continue executing without waiting for the socket operations to complete. When this is set, calls to accept() will return -1 immediately (also read and write on the socket will return immediately if there is no data available, rather than blocking until data is received or sent.)
		throw(std::runtime_error("could not set option O_NONBLOCK for socket"));
	if (bind(listenSocketFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)//The bind function is used to associate a socket with a specific local address and port number. 2nd arg is a pointer to a sockaddr structure that contains the address to which the socket should be bound. 
		throw(std::runtime_error("could not bind server socket to its address and port"));
	if (listen(listenSocketFd, SOMAXCONN) == -1)//The listen function marks a socket as passive, meaning it will be used to accept incoming connection requests. 2nd arg is an int that defines the maximum length of the queue for pending connections (specifies how many connection requests can be queued up while the server is busy handling other connections)
		throw(std::runtime_error("server can not listen on socket "));
	std::cout << "Server listening on port " << this->port << std::endl;
	this->connectedSocket.fd = listenSocketFd; // listenSocketFd is the first fd in the vector, since it must be polled for incoming connections which will be accepted and added to the vector as a new connected socket
	this->connectedSocket.events = POLLIN;
	this->connectedSocket.revents = 0;
	this->fds.push_back(this->connectedSocket);//adds a deep copy of the pollfd struct to the vector (so this connectedSocket pollfd struct is reused -overwritten- each time a new client connects and must be added to fds vector to be polled)

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
		if(poll(&fds[0],fds.size(),-1) == -1  && Server::signalReceived == false)//3rd arg of poll() is the timeout in milliseconds (-1 means infinite timeout, that is to say, poll will block indefinitely until an event occurs!!!!). We need to check on signalReceived because poll is blocked waiting for a fd to contain something to read and if SIGINT or SIGQUIT is received poll will return -1 (error) 
			throw(std::runtime_error("server could not poll sockets fd's"));
		for (size_t i = 0; i < this->fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)//bitwise AND - if evaluates to true only when a POLLIN event occurred on this fd
			{
				if (fds[i].fd == listenSocketFd)//socket with event is the server's listen socket, so there is an incoming connection
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
	int connectedSocketFd = accept(this->listenSocketFd, (sockaddr *)&clientAddress, &clientAddressLength);//accepts an incoming connection on a listening socket. The accept function creates a new socket for each incoming connection, allowing the server to communicate with multiple clients simultaneously.2nd arg will be filled with the address of the connecting client
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
	clients.push_back(Client(connectedSocketFd, *this)); // Adds new accepted client to the end of the vector
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
	char	buffer[1024];
	ssize_t	bytesRead =recv(clientConnectedfd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead < 0) // Error handling
		throw std::runtime_error("Server could not read incoming mesage ");
	else if (bytesRead == 0) { // Client has closed the connection
		std::cout << "Client has closed the connection" << std::endl;
		disconnectClient(clientConnectedfd);
		return ;
	}
	buffer[bytesRead] = '\0'; // Null-terminate the buffer

	// Check if we have an active buffer for the client; create one if not
	if (clientBuffers.find(clientConnectedfd) == clientBuffers.end())
		clientBuffers[clientConnectedfd] = ""; // Initialise to ""
	// Add what was just read from socket to the client buffer in the map
	clientBuffers[clientConnectedfd].append(buffer);

	// Read until \r\n has been found
	size_t	pos;
	while (Client::findClientByFd(clientConnectedfd, clients) && (pos = clientBuffers[clientConnectedfd].find("\r\n")) != std::string::npos) {
		std::string message = clientBuffers[clientConnectedfd].substr(0, pos);
		clientBuffers[clientConnectedfd].erase(0, pos + 2);  // Remove the processed part from the buffer
		// Process our complete command
		// Debug print
		// std::cout << "Message: " << message << std::endl;
		processMessage(clientConnectedfd, message);
	}
	// Debug print
	// printClients();
}

/**THIS IS THE OLD PROCESSMESSAGE FUNCTION. IT IS BEING REDONE BELOW, MOVING THE SPLIT TO TOP AND PASSING THE VECTOR TO EVERY HANDLE____CMD FUNCTION
 * @brief	processes the received message and divides the string in a vector 
 * 			of strings. Message is a potential irc command, a substring read 
 * 			from the socket and delimited by \r\n 
 * @param	int fd socket fd where the message came from
 * @param	std::string received message (one of the potemtial irc commands read
 * 			in fd socket)


void	Server::processMessage(int fd, std::string message) {
	std::string trimmedMsg = trimMessage(message);
	// Debug print
	std::cout << "Trimmed message from fd "<< fd << ": " << trimmedMsg << std::endl;
	Client	*client = Client::findClientByFd(fd, clients);
	// If the client is not verified, check for PASS command
	if (client->isVerified() == false) {
		if (trimmedMsg.substr(0, 7) == "CAP LS"  || trimmedMsg.substr(0, 7) == "cap ls")
			return ;
		if (trimmedMsg.substr(0, 4) == "PASS" || trimmedMsg.substr(0, 4) == "pass") {
			std::string pwd = trimmedMsg.substr(4);
			if (!pwd.empty() && !std::isspace(pwd.at(0)))//the char after PASS was not a space, so it is not a PASS command
				return;
			if (pwd.empty()) {// there was only spaces after PASS
					Handler::sendResponse(Handler::prependMyserverName(client->getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " PASS " + ERR_NEEDMOREPARAMS + "\n", fd);
					return ;
			}
			if (trimMessage(pwd) == this->password) {
				client->setVerified(true);
				std::cout << "Client with fd " << fd << " password correct!" << std::endl;
			} else
				std::cout << "Unauthorized client attempting connection" << std::endl;
		}
	}
	if (client->isRegistered() == false && client->isVerified()) {
		if (trimmedMsg.substr(0, 4) == "NICK" || trimmedMsg.substr(0, 4) == "nick")
			Handler::handleNickCmd(trimmedMsg.substr(4), *client);
		// Parse USER. To do: Double check to protect from segfaults
		if ((trimmedMsg.substr(0, 5) == "USER " || trimmedMsg.substr(0, 5) == "user ") && !client->getNickname().empty()) {
			// Debug print
			// std::cout << "Got this user: " << client->getUsername() << std::endl;
			Handler::handleUserCmd(trimmedMsg.substr(5), *client);
		}
		if (!client->getNickname().empty() && !client->getUsername().empty()) {
			std::cout << "Setting registered to true" << std::endl;
			std::string	welcomeMsg = ":" + std::string(SERVER_NAME) + " 001 " + client->getNickname() + " :Welcome to our IRC network " + client->getNickname() + "!\r\n";
			client->setRegistered(true);
			Handler::sendResponse(welcomeMsg, client->getSocketFd());
			std::string pingMsg = "PING :" + std::string(SERVER_NAME) + "\r\n";
			// Send ping
			Handler::sendResponse(pingMsg, client->getSocketFd());
		}
	} else if (client->isRegistered() && client->isVerified()) {
		// Divide received message in a vector of strings
		std::vector<std::string> divMsg = splitCmd(trimmedMsg);
		// Forward command to handler
		handler.parseCommand(divMsg, *client, clients);
	}
} */

void toLowerCase(std::string& str) {
	for (size_t i = 0; i < str.size(); ++i) {
		str[i] = std::tolower(str[i]);  // Convert each character to lowercase
	}
}

/**THIS IS THE NEW PROCESSMESSAGE FUNCTION. 
 * @brief	processes the received message and divides the string in a vector 
 * 			of strings. Message is a potential irc command, a substring read 
 * 			from the socket and delimited by \r\n 
 * @param	int fd socket fd where the message came from
 * @param	std::string received message (one of the potemtial irc commands read
 * 			in fd socket)
*/

void	Server::processMessage(int fd, std::string message) {

	//std::string trimmedMsg = trimMessage(message);
	// Debug print
	std::cout << "potential irc command from fd "<< fd << ": " << message << std::endl;
	std::vector<std::string> divMsg = splitCmd(message);
	if (divMsg.empty())
		return;
	Client	*client = Client::findClientByFd(fd, clients);
	toLowerCase(divMsg[0]);
	// If the client is not verified, check for PASS command
	if (client->isVerified() == false) {
		if (divMsg[0] == "cap") {//skip cap ls command
			if(divMsg.size() > 1) {
				toLowerCase(divMsg[1]); 
				if (divMsg[1] == "ls") {
					return ;
				}
			}
		}
		if (divMsg[0] == "pass") {
			if (divMsg.size() != 2) {// there was not a single password supplied (irc protocol does not allow whitespaces in a password)
				Handler::sendResponse(Handler::prependMyserverName(client->getSocketFd()) + ERR_NEEDMOREPARAMS_CODE + " PASS " + ERR_NEEDMOREPARAMS + "\n", fd);
				return ;
			}
			if (divMsg[1] == this->password) {
				client->setVerified(true);
				std::cout << "Client with fd " << fd << " password correct!" << std::endl;
			} else {
				Handler::sendResponse(Handler::prependMyserverName(client->getSocketFd()) + ERR_PASSWDMISMATCH_CODE + ERR_PASSWDMISMATCH + "\n", fd);
				return ;
			}
		}
	}
 	if (client->isRegistered() == false && client->isVerified()) {
		if (divMsg[0] == "nick")
			Handler::handleNickCmd(divMsg, *client);
		if (divMsg[0] == "user")
			Handler::handleUserCmd(divMsg, *client);
		if (!client->getUsername().empty() && !client->getNickname().empty()) {
			Handler::sendResponse(Handler::prependMyserverName(fd) + " 001 " + client->getNickname() + ":Welcome to our IRC network, " + client->getNickname() + "\n", fd);
			Handler::sendResponse("PING " + Handler::prependMyserverName(fd) + "\n", fd);
		} 
	} else if (client->isRegistered() && client->isVerified()) {
		if ((divMsg[0] == "user" || divMsg[0] == "pass") && client->isRegistered()) {
			Handler::sendResponse(Handler::prependMyserverName(fd) + ERR_ALREADYREGISTERED_CODE + ERR_ALREADYREGISTERED + "\n", fd);
			return ;
		}
		// Forward command to handler
		handler.parseCommand(divMsg, *client, clients);
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

	// The >> operator is used to extract data from the stream (ss) and asign it to a variable,
	// it reads characters from the stream until it encounters a whitespace character
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
	// Remove client from the clients vector
	for (size_t i = 0; i < clients.size(); i++) {
		if (clients[i].getSocketFd() == clientConnectedfd) {
			clients.erase(clients.begin() + i);
			break;
		}
	}
	// Remove client fd from the fds vector
	for (size_t i = 0; i < this->fds.size(); i++)
	{
		if (this->fds[i].fd == clientConnectedfd)
		{
			this->fds.erase(this->fds.begin() + i);
			break;
		}
	}
	// Remove client buffer from the buffers map
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

	// Check if the string contains only whitespace
	if (first == std::string::npos || last == std::string::npos) {
		return "";
	} else {
		return str.substr(first, last - first + 1);  // Return trimmed string, without starting or ending whitespaces
	}
}