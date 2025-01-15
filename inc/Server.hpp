/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igcastil <igcastil@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/24 18:35:03 by igcastil          #+#    #+#             */
/*   Updated: 2025/01/15 02:54:50 by igcastil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "../inc/Client.hpp"
#include "../inc/Handler.hpp"
#include "../inc/Channel.hpp"
#include <string>
#include <netinet/in.h>
#include <vector>
#include <poll.h> // for struct pollfd
#include <map>
#include <sstream>

# define SERVER_NAME "127.0.0.1"

class Server
{
	private:
		std::list<Client>	clients;
		Handler				handler;
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
									//					int fd; File descriptor to poll
									//					short events; Types of events poller cares about.(common event flags include POLLIN: Data can be read without blocking. POLLOUT: Normal data can be written without blocking. POLLERR: An error has occurred on the file descriptor.
									//					short revents; (output parameter, filled by the kernel with the events that actually occurred)
		std::map<int, std::string> clientBuffers;// key is client connected socket fd and value is the buffer where every read of the socket is appended

	public:
		static bool signalReceived;
		Server();
		std::list<Client> getClients(void) const;
		void closeFds();
		void initSocket();
		void init(int port, std::string pass);
		static void signalHandler(int signum);
		void acceptClient();
		void readFromFd(int fd);
		void printClients(void) const;
		static std::string trimMessage(std::string str);
		void disconnectClient(int clientConnectedfd);
		void processMessage(int fd, std::string message);
		std::vector<std::string> splitCmd(std::string trimmedMsg);
};

#endif
