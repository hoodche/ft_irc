
#ifndef HANDLER_HPP
# define HANDLER_HPP

# include "../inc/Client.hpp"
# include "../inc/Channel.hpp"
// #include "../inc/utils.hpp"
# include <sys/socket.h>
# include <iostream>
# include <algorithm>
# include <string>
# include <vector>
# include <set>
# include <map>
# include <list>

typedef void (*cmdHandler)(std::vector<std::string>, Client &);

class Handler {
	private:
		std::map<std::string, cmdHandler> cmdMap;
		static std::list<Channel> channels;

		void										initCmdMap(void);
		static void									joinCmdExec(std::map<std::string, std::string> channelDictionary, Client &client);
		static void									createChannel(std::string channelName, Client &client);
		static void									addClientToChannel(Channel &channel, Client &client);
		static std::vector<std::string>				getChannelVector(std::string channelString);
		static std::vector<std::string>				getPassVector(std::string channelString);
		static std::map<std::string, std::string>	createDictionary(std::vector<std::string> &channelVector, std::vector<std::string> &passVector);	
		static std::list<Channel>::iterator			findChannel(const std::string &channelName);
		static std::string							vectorToString(std::vector<std::string> vectorTopic, char delim);
	//it is common to all the instances

	public:
		Handler(void);
		void parseCommand(std::string input, std::list<Client> &clients, int fd);

		// Utils
		static void sendResponse(std::string message, int clientFd);
		static std::string toUpperCase(std::string str);

		// Methods for pointers to function
		static void handleUserCmd(std::string input, Client &client);
		static void handleNickCmd(std::string input, Client &client);
		static void handlePingCmd(std::string input, Client &client);
		static void	handleJoinCmd(std::vector<std::string>input, Client &client);
		static void handleTopicCmd(std::vector<std::string> input, Client &client);
		static void handleKickCmd(std::vector<std::string> input, Client &client);
};

#endif

/* Everything is static because we use function pointers, so they are not dependent of a single instance */
