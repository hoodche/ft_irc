#include "../inc/Channel.hpp"

//Init all the possible modes that the subject requires. 
//TODO: Chech how our client default modes
//Default constructor. It is defined because it is a standard, but never used

Channel::Channel(void){
	this->name = "";
	this->topic = "";
	this->password = "";
	this->userLimit = 0; //On 0, there is no limit
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
	this->userLimit = 0; // On 0, there is no limit // On 0, there is no limit
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

bool	Channel::getMode(std::string type) const {
	return modes.at(type);
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
	std::cerr << "Client is not an operator" << std::endl;
}

void Channel::addUser(Client &client)
{
	//Add something to return if we get a repeated user
	users.push_back(&client);
}

void Channel::addOperator(Client &client)
{
	operators.push_back(&client);
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
	throw std::out_of_range("Invalid Client");
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
	throw std::out_of_range("Invalid Operator Client");
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
	throw std::out_of_range("Invalid User Client");
}

void	Channel::removeClient(std::string &clientStr)
{
	if (operators.empty())
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

	if (users.empty())
		return;

	itClients = users.begin();
	while (itClients != users.end())
	{
		if ((*itClients)->getNickname() == clientStr)
		{
			operators.erase(itClients);
			return;
		}
		itClients++;
	}
	throw std::out_of_range("Invalid Client");
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
