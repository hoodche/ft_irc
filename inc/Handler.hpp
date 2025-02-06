#ifndef HANDLER_HPP
# define HANDLER_HPP

# include "../inc/Client.hpp"
# include "../inc/Channel.hpp"
# include <netinet/in.h>
# include <algorithm>
# include <string>
# include <vector>
# include <sstream>
# include <map>
# include <list>

# define USERLEN	12
# define RPL_WELCOME_CODE			"001 "


# define RPL_CHANNELMODEIS_CODE		"324 "
# define RPL_NOTOPIC_CODE			"331 "
# define RPL_NOTOPIC				":No topic is set"
# define RPL_TOPIC_CODE				"332 "
# define RPL_INVITING_CODE			"341 "
# define RPL_NAMREPLY_CODE			"353 "
# define RPL_ENDOFNAMES				"366 "

# define ERR_NOSUCHNICK_CODE		"401 "
# define ERR_NOSUCHNICK				":No such nick/channel"
# define ERR_NOSUCHCHANNEL_CODE		"403 "
# define ERR_NOSUCHCHANNEL			":No such channel"
# define ERR_TOOMANYTARGETS_CODE	"407 "
# define ERR_TOOMANYTARGETS			":No message delivered"
# define ERR_NORECIPIENT_CODE		"411 "
# define ERR_NORECIPIENT			":No recipient given"
# define ERR_NOTEXTTOSEND_CODE		"412 "
# define ERR_NOTEXTTOSEND			":No text to send"
# define ERR_UNKNOWNCOMMAND_CODE	"421 "
# define ERR_UNKNOWNCOMMAND 		":Unknown command"
# define ERR_NONICKNAMEGIVEN_CODE	"431 "
# define ERR_NONICKNAMEGIVEN		":No nickname given"
# define ERR_ERRONEUSNICKNAME_CODE	"432 "
# define ERR_ERRONEUSNICKNAME		":Erroneous nickname"
# define ERR_NICKNAMEINUSE_CODE		"433 "
# define ERR_NICKNAMEINUSE			":Nickname is already in use"
# define ERR_USERNOTINCHANNEL_CODE	"441 "
# define ERR_USERNOTINCHANNEL		":They aren't on that channel"
# define ERR_NOTONCHANNEL_CODE		"442 "
# define ERR_NOTONCHANNEL			":You're not on that channel"
# define ERR_USERONCHANNEL_CODE		"443 "
# define ERR_USERONCHANNEL			":is already on channel"
# define ERR_NEEDMOREPARAMS_CODE	"461 "
# define ERR_NEEDMOREPARAMS			":Not enough parameters"
# define ERR_ALREADYREGISTERED_CODE	"462 "
# define ERR_ALREADYREGISTERED		":You may not reregister"
# define ERR_PASSWDMISMATCH_CODE	"464 "
# define ERR_PASSWDMISMATCH			":Password incorrect"
# define ERR_KEYSET_CODE			"467 "
# define ERR_KEYSET					":Channel key already set"
# define ERR_CHANNELISFULL_CODE		"471 "
# define ERR_CHANNELISFULL			":Cannot join channel (+l)"
# define ERR_UNKNOWNMODE_CODE		":472 "
# define ERR_UNKNOWNMODE			":is unknown mode char to me"
# define ERR_INVITEONLYCHAN_CODE	"473 "
# define ERR_INVITEONLYCHAN			":Cannot join channel (+i)"
# define ERR_BADCHANNELKEY_CODE		"475 "
# define ERR_BADCHANNELKEY			":Cannot join channel (+k)"
# define ERR_CHANOPRIVSNEEDED_CODE	"482 "
# define ERR_CHANOPRIVSNEEDED		":You're not channel operator"

# define PLUS_STATUS					1
# define MINUS_STATUS					2
# define ARGV_STATUS					3

typedef void (*cmdHandler)(std::vector<std::string>, Client &);
typedef bool (*modeHandler)(Channel &, std::string);
typedef bool (*modeHandlerNoArgv)(Channel &);

class Handler {
	private:
		std::map<std::string, cmdHandler>				cmdMap;

		static std::map<std::string, modeHandler>		cmdModeMap;
		static std::map<std::string, modeHandlerNoArgv>	cmdModeMapNoArgv;
		static std::list<Channel>						channels;

		void										initCmdMap(void);
		void										initModeCmdMaps(void);
		static void									joinCmdExec(std::map<std::string, std::string> channelDictionary, Client &client);
		static void									createChannel(std::string channelName, Client &client);
		static void									addClientToChannel(Channel &channel, Client &client);
		static std::vector<std::string>				getChannelVector(std::string channelString, Client &client);
		static std::vector<std::string>				getPassVector(std::string channelString);
		static std::map<std::string, std::string>	createDictionary(std::vector<std::string> &channelVector, std::vector<std::string> &passVector);	
		static std::list<Channel>::iterator			findChannel(const std::string &channelName);
		static std::string							vectorToString(std::vector<std::string> vectorTopic, char delim);
		static std::string							createKickMessage(std::vector<std::string> &input);
		static bool									isCharInStr(std::string const &ref, const char &c);
		static void									authClientToChannel(Channel &channel, std::string &password, Client &client);
		static void									sendChannelModeIs(Client &client, Channel &channel);
		static std::string							getClientPrefix(Client const &client);
		static void									sendMsgClientsInChannel(Channel &channel, Client &client, std::string cmd, std::string argv);
		static void									sendMsgClientsInChannelKick(Channel &channel, Client &client, std::string cmd, std::string kickedClient, std::string argv);
		static void									sendMsgClientsInChannelNoPrintCh(Channel &channel, Client &client, std::string cmd, std::string argv);
		static int									getStatusSymbol(std::string str);
		static bool									parseFlagString(std::vector<std::string> &flagVector, std::string flags, Client &client);
		static void									appendToFlagStr(int &status, int &newStatus, std::string &flag, std::string &flagSendStr);
		static std::string							getAllClientsInChannel(Channel &channel);
		static void									leaveAllChannels(Client &client);
		
	//it is common to all the instances

	//Mode Function Pointers
		static bool	activateInviteMode(Channel &channel);
		static bool	deactivateInviteMode(Channel &channel);
		static bool	activateTopicPrivMode(Channel &channel);
		static bool	deactivateTopicPrivMode(Channel &channel);
		static bool	deactivateUserLimitMode(Channel &channel);

		static bool	activateUserLimitMode(Channel &channel, std::string newLimit);
		static bool	activatePasswordMode(Channel &channel, std::string newPassword);
		static bool	deactivatePasswordMode(Channel &channel, std::string newPassword);
		static bool	activateOperatorMode(Channel &channel, std::string targetClient);
		static bool	deactivateOperatorMode(Channel &channel, std::string targetClient);

	// Nick utils
		static bool	isNicknameValid(std::string nickname);
		static bool	isNicknameInUse(std::string nickname, Client *client);

	public:
		Handler(void);
		void parseCommand(std::vector<std::string> divMsg, Client &client);

		static void	deleteChannel(std::list<Channel> &channels, std::string channelName);
		static std::list<Channel>& getChannels();

		// Utils
		static std::string prependMyserverName(int clientFd);
		static void sendResponse(std::string message, int clientFd);

		// Methods for Auth Functions
		static void handleUserCmd(std::vector<std::string> input, Client &client);
		static void handleNickCmd(std::vector<std::string> input, Client &client);
		static void	handleJoinCmd(std::vector<std::string>input, Client &client);
		static void handleTopicCmd(std::vector<std::string> input, Client &client);
		static void handleKickCmd(std::vector<std::string> input, Client &client);
		static void handlePingCmd(std::vector<std::string> input, Client &client);
		static void handleModeCmd(std::vector<std::string> input, Client &client);
		static void	handleInviteCmd(std::vector<std::string> input, Client &client);
		static void	handlePrivmsgCmd(std::vector<std::string> input, Client &client);
		static void	handleQuitCmd(std::vector<std::string> input, Client &client);
		static void handlePartCmd(std::vector<std::string> input, Client &client);
};

#endif

/* Everything is static because we use function pointers, so they are not dependent of a single instance */
