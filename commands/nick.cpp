#include <string>

bool isNicknameValid(std::string nickname)
{
	// Check length (1 to 9 characters)
	if (nickname.empty() || nickname.length() > 9)
		return false;
	// Check the first character (no digits nor - from the allowed set of chars)
	char c = nickname[0];
	if (!(std::isalpha(c) || c == '_' || c == '[' || c == ']' || c == '\\' || c == '{' || c == '}' || c == '|'))
		return false;
	// Check the rest of the characters
	for (size_t i = 1; i < nickname.length(); ++i) {
		if (!(std::isalnum(nickname[i]) || nickname[i] == '_' || nickname[i] == '-' || nickname[i] == '[' || nickname[i] == ']' || nickname[i] == '\\' || nickname[i] == '{' || nickname[i] == '}' || nickname[i] == '|'))
			return false;
	}
	return true;
}