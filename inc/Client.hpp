
#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <iostream>
# include <vector>

class Channel;
class Server;

class Client {
	private:
		int 					fd;
		bool					verified;
		bool					registered;
		std::string				nick;
		std::string				username;
		std::vector<Channel *>	clientChannels;
		const Server			*connectedToServer;

	public:
		Client(void);
		Client(int receivedFd, const Server &connected2Serv);
		~Client(void);

		// Getters
		int	getSocketFd(void) const;
		std::string	getNickname(void) const;
		std::string getUsername(void) const;
		const Server* getServer(void) const;
		bool isVerified(void) const;
		bool isRegistered(void) const;

		// Setters
		void setNickname(std::string nickname);
		void setUsername(std::string user);
		void setVerified(bool tf);
		void setRegistered(bool tf);

		// Methods
		static Client	*findClientByFd(int fd, std::vector<Client> &clients);
		bool			isClientInChannel(std::string channelName);
		void			addClientChannel(Channel &channel);
};

#endif
