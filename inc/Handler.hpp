
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
# include <sstream>
# include <set>
# include <map>

# define USERLEN	12
# define ERR_NEEDMOREPARAMS		":Not enough parameters" //  "<client> <command> :Not enough parameters"


# define ERR_NONICKNAMEGIVEN_CODE	"431 "
# define ERR_NONICKNAMEGIVEN		":No nickname given"
# define ERR_ERRONEUSNICKNAME_CODE	"432 "
# define ERR_ERRONEUSNICKNAME		":Erroneous nickname"
# define ERR_NICKNAMEINUSE_CODE		"433 "
# define ERR_NICKNAMEINUSE			":Nickname is already in use"
# define ERR_NEEDMOREPARAMS_CODE	"461 "
# define ERR_NEEDMOREPARAMS			":Not enough parameters"
# define ERR_ALREADYREGISTERED_CODE	"462 "
# define ERR_ALREADYREGISTERED		":You may not reregister"
# define ERR_PASSWDMISMATCH_CODE	"464 "
# define ERR_PASSWDMISMATCH			":Password incorrect"
# define ERR_NICKNAMEINUSE			":Nickname is already in use"

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
		void parseCommand(std::vector<std::string> divMsg, Client &client, std::vector<Client> &clients);

		// Utils
		static std::string prependMyserverName(int clientFd);
		static void sendResponse(std::string message, int clientFd);

		// Methods for Auth Functions
		static void handleUserCmd(std::vector<std::string> divMsg, Client &client);
		static void handleNickCmd(std::vector<std::string> divMsg, Client &client);

		static void handlePingCmd(std::vector<std::string> input, Client &client);
};

#endif

/* Everything is static because we use function pointers, so they are not dependent of a single instance */
