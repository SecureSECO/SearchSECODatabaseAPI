/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once

#include <string>
#include <vector>

/// <summary>
/// Implements generic functionality.
/// </summary>
class Utility
{
public:
	/// <summary>
	/// Safely attempts to convert a string to a corresponding int.
	/// </summary>
	/// <param name="str">
	/// The string to convert.
	/// </param>
	/// <returns>
	/// int corresponding to string on success, or 0 when the string cannot be converted.
	/// If the string cannot be converted, errno is set to EDOM. If the converted string would fall
	/// out of the range of an int, errno is set to ERANGE. Strings of the form "123AAA" are parsed
	/// to 123.
	/// </returns>
	static int safeStoi(std::string str);

	/// <summary>
	/// Safely attempts to convert a string to a corresponding long long.
	/// </summary>
	/// <param name="str">
	/// The string to convert.
	/// </param>
	/// <returns>
	/// long long corresponding to string on success, or 0 when the string cannot be converted.
	/// If the string cannot be converted, errno is set to EDOM. If the converted string would fall
	/// out of the range of a long long, errno is set to ERANGE. Strings of the form "123AAA" are
	/// parsed to 123.
	/// </returns>
	static long long safeStoll(std::string str);

	/// <summary>
	/// Appends 'result' by a string, and adds a special character ('delimiter') at the end.
	/// </summary>
	static void appendBy(std::vector<char>& result, std::string word, char delimiter);

	/// <summary>
	/// Appends 'result' by a multiple strings while separating the different strings
	/// by a special character ('wordSeparator') and adds a special character at the end ('endChar').
	/// </summary>
	static void appendBy(std::vector<char> &result, std::vector<std::string> words, char wordSeparator, char endChar);

	/// <summary>
	/// Makes a tuple of the words provided as input by putting commas between different words and
	/// encapsulating the separated words by parentheses.
	/// </summary>
	/// <returns>
	/// Tuple consisting of the words provided as input.
	/// </returns>
	static std::string makeTuple(std::vector<std::string> words);

	/// <summary>
	/// Splits a string ('str') on a special character ('delimiter').
	/// </summary>
	/// <returns>
	/// The vector consisting of the substrings. A delimiter at the end of the string is ignored.
	/// </returns>
	static std::vector<std::string> splitStringOn(std::string str, char delimiter);
};
