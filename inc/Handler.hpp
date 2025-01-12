
#ifndef HANDLER_HPP
# define HANDLER_HPP

# include "../inc/Client.hpp"
# include "../inc/Channel.hpp"
#include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h> 
# include <iostream>
# include <algorithm>
# include <string>
# include <string.h> //for memset
# include <vector>
# include <set>
# include <map>


# define ERR_NONICKNAMEGIVEN_CODE	"431 "
# define ERR_NONICKNAMEGIVEN		":No nickname given"
# define ERR_ERRONEUSNICKNAME_CODE	"432 "
# define ERR_ERRONEUSNICKNAME		":Erroneous nickname"
# define ERR_NEEDMOREPARAMS_CODE	"461"
# define ERR_NEEDMOREPARAMS			":Not enough parameters"
# define ERR_NICKNAMEINUSE			":Nickname is already in use"
// To do: See if implementing ERR_NICKCOLLISION is needed to be implemented

typedef void (*cmdHandler)(std::string, Client &);

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
		void parseCommand(std::vector<std::string> divMsg, Client &client, std::vector<Client> &clients);

		// Utils
		static std::string prependMyserverName(int clientFd);
		static std::string composeResponse(std::string field1, std::string field2, std::string field3, int clientFd) ;
		static void sendResponse(std::string message, int clientFd);
		static std::string toUpperCase(std::string str);

		// Methods for pointers to function
		static void handleUserCmd(std::string input, Client &client);
		static void handleNickCmd(std::string input, Client &client);
		static void handlePingCmd(std::string input, Client &client);
};

#endif

/* Everything is static because we use function pointers, so they are not dependent of a single instance */
