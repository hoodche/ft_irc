/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igcastil <igcastil@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/24 18:26:33 by igcastil          #+#    #+#             */
/*   Updated: 2024/12/28 12:23:05 by igcastil         ###   ########.fr       */
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
	this->serverAddress.sin_addr.s_addr = INADDR_ANY; //This macro allows the socket to bind to all available interfaces on the host machine. It is typically used in server applications to accept connections from any IP address.
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
		if(poll(&fds[0],fds.size(),-1) == -1)//3rd arg of poll() is the timeout in milliseconds (-1 means infinite timeout, that is to say, poll will block indefinitely until an event occurs!!!!)
			throw(std::runtime_error("server could not poll sockets fd's"));
		for (size_t i = 0; i < this->fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)//bitwise AND - if evaluates to true only when a POLLIN event occurred on this fd
			{
				if (fds[i].fd == listenSocketFd)//socket with event is the server's listen socket, so there is an incoming connection
					this->accept_new_client();
				//else
					//read data from client
			}
		}
	}
	this->closeFds();
}
void Server::accept_new_client()
{
	//Client cli;
	memset(&clientAddress, 0, sizeof(clientAddress));
	socklen_t clientAddressLength = sizeof(clientAddress);
	int connectedSocketFd = accept(this->listenSocketFd, (sockaddr *)&(clientAddress), &clientAddressLength);
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
/* 	cli.SetFd(incofd);
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));
	clients.push_back(cli); */
	
	std::cout << "a new client has been connected with socket fd " << connectedSocketFd << std::endl;
}