#include "../inc/Channel.hpp"

//Init all the possible modes that the subject requires. 
//Default constructor. It is defined because it is a standard, but never used

Channel::Channel(void){
	this->name = "";
	this->topic = "";
	this->password = "";
	this->userLimit = 0;
	this->isTopicMode = false;
	this->isInviteMode = false;
}

// Usable constructor. We init the instance using a Client. Following the rfc, this 
// user is the default operator

Channel::Channel(Client &firstOperator){
	this->name = "";
	this->topic = "";
	this->password = "";
	this->isTopicMode = false;
	this->isInviteMode = false;
	this->userLimit = 0;
	this->operators.push_back(&firstOperator);
}

Channel::~Channel(void){}

std::string Channel::getName(void) const
{
	return this->name;
}

std::string Channel::getTopic(void) const
{
	return this->topic;
}

std::vector<Client *> Channel::getOperators(void) const
{
	return this->operators;
}

std::vector<Client *> Channel::getUsers(void) const
{
	return this->users;
}

void Channel::setName(std::string const channelName)
{
	this->name = channelName;
}

void Channel::setTopic(std::string const channelTopic, Client &client)
{
	std::vector<Client *>::iterator it = operators.begin();
	while(it != operators.end())
	{
		if ((*it)->getNickname() == client.getNickname())
		{
			this->topic = channelTopic;
			return;
		}
		it++;
	}
	return;
}

void Channel::addUser(Client &client)
{
	users.push_back(&client);
	return ;
}

void Channel::addOperator(Client &client)
{
	operators.push_back(&client);
	return ;
}

Client	*Channel::getClient(std::string &clientStr)
{
	std::vector<Client*>::iterator itClients = operators.begin();

	if (!operators.empty())
	{
		while (itClients != operators.end())
		{
			if ((*itClients)->getNickname() == clientStr)
				return (*itClients);
			itClients++;
		}
	}

	itClients = users.begin();

	if (!users.empty())
	{
		while (!users.empty() && itClients != users.end())
		{
			if ((*itClients)->getNickname() == clientStr)
				return (*itClients);
			itClients++;
		}
	}
	return (NULL);
}

Client	*Channel::getOperatorClient(std::string &clientStr)
{
	std::vector<Client*>::iterator itClients = operators.begin();

	if (!operators.empty())
	{
		while (itClients != operators.end())
		{
			if ((*itClients)->getNickname() == clientStr)
				return (*itClients);
			itClients++;
		}
	}
	return (NULL);
}

Client	*Channel::getUserClient(std::string &clientStr)
{
	std::vector<Client*>::iterator itClients = users.begin();

	if (!users.empty())
	{
		while (!users.empty() && itClients != users.end())
		{
			if ((*itClients)->getNickname() == clientStr)
				return (*itClients);
			itClients++;
		}
	}
	return (NULL);
}

void	Channel::removeClient(const std::string &clientStr)
{
	if (operators.empty() && users.empty())
		return;

	std::vector<Client*>::iterator itClients = operators.begin();
	while (itClients != operators.end())
	{
		if ((*itClients)->getNickname() == clientStr)
		{
			operators.erase(itClients);
			return;
		}
		itClients++;
	}

	itClients = users.begin();
	while (itClients != users.end())
	{
		if ((*itClients)->getNickname() == clientStr)
		{
			users.erase(itClients);
			return;
		}
		itClients++;
	}
}

bool Channel::isClientOperator(Client &client)
{
	std::vector<Client *>::iterator itOp = operators.begin();
	while (itOp != operators.end())
	{
		if (client.getNickname() == (*itOp)->getNickname())
			return true;
		itOp++;
	}
	return false;
}

bool Channel::getTopicMode(void) const
{
	return (this->isTopicMode);
}

bool Channel::getInviteMode(void) const
{
	return (this->isInviteMode);
}

unsigned int Channel::getUserLimit(void) const
{
	return (this->userLimit);
}

std::string	Channel::getPassword(void) const
{
	return (this->password);
}

void	Channel::setTopicMode(bool state)
{
	this->isTopicMode = state;
}

void	Channel::setInviteMode(bool state)
{
	this->isInviteMode = state;
}

void	Channel::setUserLimit(unsigned int limit)
{
	this->userLimit = limit;
}

void	Channel::setPassword(std::string newPass)
{
	this->password = newPass;
}
