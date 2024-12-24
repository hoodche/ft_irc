/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igcastil <igcastil@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/24 18:35:03 by igcastil          #+#    #+#             */
/*   Updated: 2024/12/25 00:08:11 by igcastil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server
{
private:
	int port;
	std::string password;
	int socketFd;
	struct sockaddr_in serverAddressStruct;	//struct sockaddr_in {
									//	sa_family_t     sin_family;     / is an unsigned short int
									//	in_port_t       sin_port;       / is an unsigned short int
									//	struct in_addr  sin_addr;       / IPv4 address */
									//}; handles IPv4 addresses The sin_port and sin_addr members are stored in network byte order.
	std::vector<Client> clients;
	std::vector<Channel> channels;
	std::vector<struct pollfd> fds;
	struct sockaddr_in cliadd;
	struct pollfd new_cli;			//struct pollfd {
									//					int fd;
									//					short events; (common event flags include POLLIN: Data other than high-priority data can be read without blocking. POLLOUT: Normal data can be written without blocking. POLLERR: An error has occurred on the file descriptor.
									//					short revents; (used by the poll function to return the events that actually occurred. This field is set by the poll function and is used to determine which file descriptors are ready for the specified operations)
									//				};represents a file descriptors to be monitored for events using the poll function.
public:
	void Server::initSocket();
	void init(int port, std::string pass);
};

#endif