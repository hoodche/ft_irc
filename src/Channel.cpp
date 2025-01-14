#include "../inc/Channel.hpp"

//Init all the possible modes that the subject requires. 
//TODO: Chech how our client default modes
void Channel::initModeMap(void){
	modes["i"] = false;
	modes["t"] = false;
	modes["k"] = false;
	modes["o"] = false;
	modes["l"] = false;
}

//Default constructor. It is defined because it is a standard, but never used

Channel::Channel(void){
	this->name = "";
	this->topic = "";
	this->password = "";
	this->userLimit = 0; //On 0, there is no limit
	initModeMap();
}

// Usable constructor. We init the instance using a Client. Following the rfc, this 
// user is the default operator

Channel::Channel(Client &firstOperator){
	this->name = "";
	this->topic = "";
	this->password = "";
	this->userLimit = 0; // On 0, there is no limit // On 0, there is no limit
	this->operators.push_back(&firstOperator);
	initModeMap();
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
