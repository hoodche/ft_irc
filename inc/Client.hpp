
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
		int 					fd;
		bool					verified;
		bool					registered;
		std::string				nick;
		std::string				username;
		std::string				realname;
		std::vector<Channel *>	invitedChannels;
		std::vector<Channel *>	clientChannels;
		std::string				ipAddr;
		const Server			*connectedToServer;

	public:
		Client(int receivedFd, const Server &newServer, std::string ip);
		~Client(void);

		// Getters
		int							getSocketFd(void) const;
		std::string					getNickname(void) const;
		std::string					getUsername(void) const;
		std::string					getRealname(void) const;
		std::string					getIpAddr(void) const;
		const Server*				getServer(void) const;
		std::vector<Channel *> 		&getClientChannels(void);
		bool						isVerified(void) const;
		bool						isRegistered(void) const;
		bool						isInvited(Channel &channel) const;
		Channel	*					getChannel(std::string &channelStr);
		// Setters
		void						setNickname(std::string nickname);
		void						setUsername(std::string user);
		void						setRealname(std::string realname);
		void						setVerified(bool tf);
		void						setRegistered(bool tf);

		void						addServer(Server &newServer);
		void						addChannel(Channel &newChannel);
		void						addInvitedChannel(Channel &invitedChannel);
		void						removeInvitation(Channel &channel);
		// Methods
		static Client				*findClientByFd(int fd, std::list<Client> &clients);
		static Client				*findClientByName(std::string name, std::list<Client> &clients);
		bool						isClientInChannel(std::string &channelName);
		
		void						removeChannel(std::string &channelStr);
		void						removeInvitedChannels(std::string &channelName);
};

#endif
