/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
� Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include <sstream>
#include "Utility.h"

int Utility::safeStoi(std::string str)
{
        errno = 0;
        int res = 0;
        try
        {
		res = stoi(str);
        }
        catch(const std::exception& e)
        {
                if (errno != ERANGE)
                {
                        errno = EDOM;
                }
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
        catch(const std::exception& e)
        {
                if (errno != ERANGE)
                {
                        errno = EDOM;
                }
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
