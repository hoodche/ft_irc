#include <string>
bool isNicknameValid(std::string nickname)
{
	/* 	if (nickname.size() < 1 || nickname.size() > 9) {
		message	= nickname + " " + ERR_ERRONEUSNICKNAME; // To do: Check if space is needed
		sendResponse(message, client.getSocketFd());
		return ;
	}
	// Check alphanumeric characters
    for (std::string::size_type i = 0; i < nickname.size(); ++i) {
        if (!std::isalnum(nickname[i])) {
            message = nickname + " " + ERR_ERRONEUSNICKNAME;
            sendResponse(message, client.getSocketFd());
            return;
        }
    } */
	(void)nickname;
	return (true);
}