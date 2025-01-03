
#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <iostream>

class Client {
	private:
		int 		fd;
		bool		registered;
		std::string	nick;
		std::string	username;
		std::string	password;

	public:
		Client(void);
		Client(int receivedFd);
		~Client(void);

		// getter
		int	getSocketFd(void) const;
};

#endif