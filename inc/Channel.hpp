#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <vector>
# include <string>
# include <map>
# include "../inc/Client.hpp"

class Channel {
	private:
		void initModeMap(void); //Method to init the mode map
		Channel();

		std::string					name;
		std::string					topic;	
		std::vector<Client *>		operators;
		std::vector<Client *>		users;
		std::map<std::string, bool>	modes;
		std::string					password;
		unsigned int				userLimit;

	public:
		Channel(Client &firstOperator);
		~Channel();
		
		
		//Getters
		std::string getName(void) const;
		std::string getTopic(void) const;
		Client*		getClient(std::string &clientStr);
	
		//Setters
		void		setName(const std::string name);
		void		setTopic(const std::string topic, Client &client);

		//Adders
		void		addUser(Client &client);
		void		addOperator(Client &client);

		void		removeClient(std::string &clientStr);
		bool		isClientOperator(Client &client);
};

#endif
