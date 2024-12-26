/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igcastil <igcastil@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/24 18:26:33 by igcastil          #+#    #+#             */
/*   Updated: 2024/12/26 11:43:49 by igcastil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include <arpa/inet.h> // for htons
#include <stdexcept>   // for std::runtime_error
#include <poll.h>      // for POLLIN
#include <fcntl.h>     // for fcntl
#include <iostream>
#include <unistd.h> // for read
#include <csignal> //for SIGINT and SIGQUIT

Server::Server() : serverSocketFd(-1), clientSocketFd(-1)
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
	if (this->serverSocketFd != -1)
		close(this->serverSocketFd);
	if (this->clientSocketFd != -1)
		close(this->clientSocketFd);
}

void Server::initSocket()
{
	int socketOptionEnable = 1;
	this->serverAddress.sin_family = AF_INET; //address family for IPv4 addresses
	this->serverAddress.sin_port = htons(this->port);//port number. The htons() stands for "Host TO Network Short." It is used to convert a 16-bit number
													//from host byte order (endianness) to network byte order. In most systems, the host byte order is little-endian,
													//however, network protocols like TCP/IP use big-endian order.
	this->serverAddress.sin_addr.s_addr = INADDR_ANY; //This macro allows the socket to bind to all available interfaces on the host machine. It is typically used in server applications to accept connections from any IP address.
	this->serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);	//SOCK_STREAM indicates that the socket type is stream-oriented, which means it provides a reliable, two-way, connection-based byte stream. This type of socket is typically used for TCP connections. TCP is a protocol that ensures data is delivered in the same order it was sent and without errors, making it suitable for applications that require reliable communication, such as web servers, email clients, and file transfer programs.
														//3rd arg is the protocol to be used with the socket. Typically, this is set to 0 to use the default protocol for the specified address family (1st arg) and socket type (2nd arg)
	if(serverSocketFd == -1)
		throw(std::runtime_error("socket could not be created "));
	if(setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &socketOptionEnable, sizeof(socketOptionEnable)) == -1)//allows to customize the behavior of a socket. 2nd arg specifies the protocol level at which the option resides (common values include SOL_SOCKET for socket-level options and IPPROTO_TCP for TCP-level options). 3rd arg specifies the option to be set (SO_REUSEADDR to allow reuse of local addresses)
		throw(std::runtime_error("could not set option SO_REUSEADDR to allow reuse of local addresses for socket"));
	//if (fcntl(serverSocketFd, F_SETFL, O_NONBLOCK) == -1)//The fcntl function is used to perform various operations on file descriptors, including sockets. One common use of fcntl is to set a socket to non-blocking mode, which allows the program to continue executing without waiting for the socket operations to complete.
	//	throw(std::runtime_error("could not set option O_NONBLOCK for socket"));
	if (bind(serverSocketFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)//The bind function is used to associate a socket with a specific local address and port number. 2nd arg is a pointer to a sockaddr structure that contains the address to which the socket should be bound. 
		throw(std::runtime_error("could not bind server socket to its address and port"));
	if (listen(serverSocketFd, SOMAXCONN) == -1)//The listen function marks a socket as passive, meaning it will be used to accept incoming connection requests. 2nd arg is an int that defines the maximum length of the queue for pending connections (specifies how many connection requests can be queued up while the server is busy handling other connections)
		throw(std::runtime_error("server can not listen on socket "));
	std::cout << "Server listening on port " << this->port << std::endl;

}


void Server::init(int port, std::string password)
{
	this->password = password;
	this->port = port;
	this->initSocket();

	//________TESTING BLOCK_________
	char buffer[1000];
	socklen_t clientAddressLength = sizeof(clientAddress);
	this->clientSocketFd = accept(serverSocketFd, (struct sockaddr *)&clientAddress, &clientAddressLength);//Await a connection on serverSocketFd. When a connection arrives, opens a new socket to communicate with it (returning its socket descriptor or -1 for errors),sets clientAddress (which is clientAddressLength bytes long) to the address of the connecting peer and clientAddressLength to the address's actual length.
	if (this->clientSocketFd < 0)
		throw(std::runtime_error("server could not accept incoming connection "));
	ssize_t bytes_read = read(this->clientSocketFd, buffer, sizeof(buffer) - 1);//Reads 3rd arg bytes into buffer from clientSocketFd. Returns the number read, -1 for errors or 0 for EOF
	if (bytes_read < 0)
		throw(std::runtime_error("server could not read incoming message "));
	buffer[bytes_read] = '\0'; // Null-terminate the buffer
	std::cout << "Received message: " << buffer << std::endl;
	//________END TESTING BLOCK_________

	while (Server::signalReceived == false)
	{
		//poll clients fds
	}
	this->closeFds();
}