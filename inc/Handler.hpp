
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

typedef void (*cmdHandler)(std::vector<std::string>, Client &);

class Handler {
	private:
		std::map<std::string, cmdHandler> cmdMap;
		static std::vector<Channel> channels;

		void			initCmdMap(void);
		static void		handleJoinCmd(std::string input, Client &client); //static cause 
		static void		joinCmdExec(std::string channelName, Client &client);
		static void		createChannel(std::string channelName, Client &client);
		static void		addClientToChannel(Channel &channel, Client &client);
	//it is common to all the instances

	public:
		Handler(void);
		void parseCommand(std::string input, std::vector<Client> &clients, int fd);

		// Utils
		static void sendResponse(std::string message, int clientFd);
		static std::string toUpperCase(std::string str);

		// Methods for pointers to function
		static void handleUserCmd(std::string input, Client &client);
		static void handleNickCmd(std::string input, Client &client);
		static void handlePingCmd(std::string input, Client &client);
};

#endif

/* Everything is static because we use function pointers, so they are not dependent of a single instance */
