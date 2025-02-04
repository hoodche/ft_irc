/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nvillalt <nvillalt@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/24 18:35:03 by igcastil          #+#    #+#             */
/*   Updated: 2025/02/04 16:42:52 by nvillalt         ###   ########.fr       */
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

# define SOCKET_SIZE	131072

class Server
{
	private:
		std::list<Client>	clients;
		Handler				handler;
		int port;
		std::string password;
		int listenSocketFd;
		struct sockaddr_in serverAddress, clientAddress;
		std::vector<struct pollfd> fds;
		struct pollfd connectedSocket;
		std::map<int, std::string> clientBuffers;

	public:
		static bool signalReceived;
		Server();
		std::list<Client> getClients(void) const;
		std::list<Client> *getClientsPtr(void);
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
