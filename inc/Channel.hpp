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

		std::string					topic;	
		std::vector<Client *>		operators;
		std::vector<Client *>		users;
		std::map<std::string, bool>	modes;
		std::string					password;
		unsigned int				userLimit;

	public:
		Channel(Client &firstOperator);
		~Channel();
};

#endif
