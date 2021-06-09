/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include "Utility.h"

int Utility::safeStoi(std::string str)
{
	errno = 0;
	int res = 0;
	try
	{
		res = stoi(str);
	}
	catch(const std::out_of_range& e)
	{
		errno = ERANGE;
	}
	catch(const std::exception& e)
	{
		errno = EDOM;
	}
	return res;
}

long long Utility::safeStoll(std::string str)
{
	errno = 0;
	long long res = 0;
	try
	{
		res = stoll(str);
	}
	catch(const std::out_of_range& e)
	{
		errno = ERANGE;
	}
	catch(const std::exception& e)
	{
		errno = EDOM;
	}
	return res;
}

void Utility::appendBy(std::vector<char>& result, std::string word, char endCharacter)
{
	for (int i = 0; i < word.size(); i++)
	{
		result.push_back(word[i]);
	}
	result.push_back(endCharacter);
}

void Utility::appendBy(std::vector<char> &result, std::vector<std::string> words, char wordSeparator, char endCharacter)
{
	for (int i = 0; i < words.size(); i++)
	{
		appendBy(result, words[i], wordSeparator);
	}

	if (words.size() != 0)
	{
		result.pop_back();
	}
	result.push_back(endCharacter);
}

std::string Utility::makeTuple(std::vector<std::string> words)
{
	std::vector<char> chars = { '(' };
	for (std::string word : words)
	{
		Utility::appendBy(chars, word, ',');
	}
	if (chars.size() > 1)
	{
		chars.pop_back();
	}
	chars.push_back(')');
	return std::string(chars.begin(), chars.end());
}

std::vector<std::string> Utility::splitStringOn(std::string str, char delimiter)
{
	std::stringstream strStream(str);
	std::string item;
	std::vector<std::string> substrings;
	while (getline(strStream, item, delimiter))
	{
		substrings.push_back(item);
	}
	return substrings;
}

std::string Utility::hashToUuidString(std::string hash)
{
	return hash.substr(0, 8) + "-" + hash.substr(8, 4) + "-" + hash.substr(12, 4) + "-" + hash.substr(16, 4) + "-" + hash.substr(20, 12);
}

std::string Utility::uuidStringToHash(std::string uuid)
{
	boost::erase_all(uuid, "-");
	return uuid;
}
