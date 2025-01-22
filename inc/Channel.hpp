#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <vector>
# include <string>
# include <map>
# include "../inc/Client.hpp"

class Channel {
	private:
		Channel();

		std::string					name;
		std::string					topic;	
		std::vector<Client *>		operators;
		std::vector<Client *>		users;
		std::map<std::string, bool>	modes;
		std::string					password;
		unsigned int				userLimit;

		bool						isTopicMode;
		bool						isInviteMode;

	public:
		Channel(Client &firstOperator);
		~Channel();
		
		//Getters
		std::string		getName(void) const;
		std::string		getTopic(void) const;
		Client*			getClient(std::string &clientStr);
		Client*			getOperatorClient(std::string &clientStr);
		Client*			getUserClient(std::string &clientStr);
		bool			getTopicMode(void) const;
		bool			getInviteMode(void) const;
		unsigned int	getUserLimit(void) const;
		std::string		getPassword(void) const;
		std::vector<Client *> getOperators(void) const;
		std::vector<Client *> getUsers(void) const;
		bool		getMode(std::string type) const;
	
		//Setters
		void	setName(const std::string name);
		void	setTopic(const std::string topic, Client &client);
		void	setTopicMode(bool state);
		void	setInviteMode(bool state);
		void	setUserLimit(unsigned int limit);
		void	setPassword(std::string newPass);

		//Adders
		void		addUser(Client &client);
		void		addOperator(Client &client);

		void		removeClient(const std::string &clientStr);
		bool		isClientOperator(Client &client);
};

#endif
