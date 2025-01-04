
#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <iostream>
#include <vector>

class Client {
	private:
		int 		fd;
		bool		registered;
		bool		oper;
		std::string	nick;
		std::string	username;
		std::string	password; // Needed?

	public:
		Client(void);
		Client(int receivedFd);
		~Client(void);

		// Getters
		int	getSocketFd(void) const;
		std::string	getNickname(void) const;
		std::string getUsername(void) const;

		// Setters
		void setNickname(std::string nickname);
		void setUsername(std::string user);

		// Methods
		static Client *findClientByFd(int fd, std::vector<Client> &clients);
};

#endif