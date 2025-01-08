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
	this->topic = "";
	this->password = "";
	this->userLimit = 0; //On 0, there is no limit
	initModeMap();
}

// Usable constructor. We init the instance using a Client. Following the rfc, this 
// user is the default operator

Channel::Channel(Client &firstOperator){
	this->topic = "";
	this->password = "";
	this->userLimit = 0; // On 0, there is no limit // On 0, there is no limit
	firstOperator.setInChannel(true);;
	this->operators.push_back(&firstOperator);
	initModeMap();
}

Channel::~Channel(void){}
std::string Channel::getName(void) const
{
	return this->name;
}

void Channel::setName(std::string const channelName)
{
	this->name = channelName;
}

void Channel::addUser(Client &client)
{
	users.push_back(&client);
}

void Channel::addOperator(Client &client)
{
	operators.push_back(&client);
}
