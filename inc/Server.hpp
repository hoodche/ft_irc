/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igcastil <igcastil@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/24 18:35:03 by igcastil          #+#    #+#             */
/*   Updated: 2024/12/28 12:10:22 by igcastil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <netinet/in.h>
#include <vector>
#include <poll.h> // for struct pollfd


class Server
{
private:
	
	int port;
	std::string password;
	int listenSocketFd;
	struct sockaddr_in serverAddress, clientAddress;	//struct sockaddr_in {
														//	sa_family_t     sin_family;     / is an unsigned short int
														//	in_port_t       sin_port;       / is an unsigned short int
														//	struct in_addr  sin_addr;       / IPv4 address */
														//}; handles IPv4 addresses The sin_port and sin_addr members are stored in network byte order.
	std::vector<struct pollfd> fds;//this vector holds all socket fds to be polled (the first one is server's listen socket -polled for incoming connections- and the rest are client connected sockets returned by the call to accept -polled for incoming data-)
	struct pollfd connectedSocket;	//struct pollfd {
								//					int fd;
								//					short events; (common event flags include POLLIN: Data other than high-priority data can be read without blocking. POLLOUT: Normal data can be written without blocking. POLLERR: An error has occurred on the file descriptor.
								//					short revents; (output parameter, filled by the kernel with the events that actually occurred)
public:
	static bool signalReceived;
	Server();
	void closeFds();
	void initSocket();
	void init(int port, std::string pass);
	static void SignalHandler(int signum);
	void accept_new_client();
};

#endif