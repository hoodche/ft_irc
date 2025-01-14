
#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <iostream>
# include <vector>
# include <list>

class Channel;
class Server;

class Client {
	private:
		//Client(void);
		int 		fd;
		bool		verified;
		bool		registered;
		std::string	nick;
		std::string	username;
		std::vector<Channel *> clientChannels;
		const Server		*server;

	public:
		Client(int receivedFd, const Server &newServer);
		~Client(void);

		// Getters
		int	getSocketFd(void) const;
		std::string	getNickname(void) const;
		std::string getUsername(void) const;
		bool isVerified(void) const;
		bool isRegistered(void) const;
		Channel	&getChannel(std::string &channelStr);
		// Setters
		void setNickname(std::string nickname);
		void setUsername(std::string user);
		void setVerified(bool tf);
		void setRegistered(bool tf);

		void addServer(Server &newServer);
		void addChannel(Channel &newChannel);
		// Methods
		static Client	*findClientByFd(int fd, std::list<Client> &clients);
		bool			isClientInChannel(std::string channelName);
};

#endif
