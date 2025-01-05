/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nvillalt <nvillalt@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/24 18:26:33 by igcastil          #+#    #+#             */
/*   Updated: 2025/01/03 22:05:34 by nvillalt         ###   ########.fr       */
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

Server::Server() : listenSocketFd(-1)
{
}
bool Server::signalReceived = false;
void Server::SignalHandler(int signal)
{
	if(signal == SIGINT)
		std::cout << std::endl << "Signal SIGINT Received!" << std::endl;
	else if(signal == SIGQUIT)
		std::cout << std::endl << "Signal SIGQUIT Received!" << std::endl;
	Server::signalReceived = true;
}

void	Server::closeFds()
{
	for (size_t i = 0; i < this->fds.size(); i++)
		close(this->fds[i].fd);
}

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
	clients.push_back(Client(connectedSocketFd)); // Adds new accepted client to the end of the vector
	std::cout << "a new client from IP "<< inet_ntoa(clientAddress.sin_addr) << " and port " << ntohs(clientAddress.sin_port) << " has been connected with socket fd " << connectedSocketFd << std::endl;
}

void Server::readFromFd(int clientConnectedfd)
{
	char buffer[1024];
	ssize_t bytesRead = recv(clientConnectedfd, buffer, sizeof(buffer) - 1 , 0);//Reads 3rd arg bytes into buffer from clientSocketFd. Returns the number read, -1 for errors or 0 for EOF.The call to recv() is blocking by default.(it will block the execution of the program until data is available to be read from the file descriptor or an error occurs. If there is no data available, the program will wait (block) until data becomes available.). But here is not blocking since clientConnectedfd was set to fcntl(connectedSocketFd, F_SETFL, O_NONBLOCK)
	if (bytesRead < 0)
		throw(std::runtime_error("server could not read incoming message "));
	else if (bytesRead == 0)// Client has closed the connection!!
	{
		// Move all of this to 1 function -> handle dc
		std::cout << "Client has closed the connection" << std::endl;
		close(clientConnectedfd);//server closes socket belonging to connection closed by client
        
		for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i].getSocketFd() == clientConnectedfd) {
                clients.erase(clients.begin() + i);
                break;
            }
        }
		
		for (size_t i = 0; i < this->fds.size(); i++)// search through fds vector to erase the closed socket from the pollfd array
		{
			if (this->fds[i].fd == clientConnectedfd)
			{
				this->fds.erase(this->fds.begin() + i);
				break;
			}
		}
	}
	else
	{
		buffer[bytesRead] = '\0'; // Null-terminate the buffer
		std::string message = trimMessage(buffer);
		Client *client = Client::findClientByFd(clientConnectedfd, clients);
		// Debug print
		std::cout << "Received message from fd " << clientConnectedfd << ": " << buffer << std::endl;
		if (client->isVerified() == false) {
			if (message.substr(0, 5) == "PASS ") {
				std::string pwd = message.substr(5);
				// std::cout << "DEBUG RAW MESSAGE: [" << message << "] (length: " << message.length() << ")" << std::endl;
				// std::cout << "Extracted PWD: [" << pwd << "], Expected PWD: [" << this->password << "]" << std::endl;
				if (pwd == this-> password) {
					client->setVerified(true);
					// Debug print	
					std::cout << "Client with fd " << clientConnectedfd << " password correct" << std::endl;
				} else {
					// Debug Print. Same as above, move to function.
					std::cout << "Unauthorized client attempting connection. Closing fd..." << std::endl;
					close(clientConnectedfd);//server closes socket belonging to connection closed by client
				
					for (size_t i = 0; i < clients.size(); i++) {
						if (clients[i].getSocketFd() == clientConnectedfd) {
							clients.erase(clients.begin() + i);
							break;
						}
					}
					
					for (size_t i = 0; i < this->fds.size(); i++)// search through fds vector to erase the closed socket from the pollfd array
					{
						if (this->fds[i].fd == clientConnectedfd)
						{
							this->fds.erase(this->fds.begin() + i);
							break;
						}
					}
				}
			} else {
				// Debug print
				this->printClients();
				handler.parseCommand(buffer, clients, clientConnectedfd);
			}
		}
	}
}

void Server::printClients() const
{
    std::cout << "Currently connected clients:" << std::endl;
    for (size_t i = 0; i < clients.size(); i++)
    {
        std::cout << "Client " << i + 1 << ": Socket FD = " << clients[i].getSocketFd() << std::endl;
    }
}

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