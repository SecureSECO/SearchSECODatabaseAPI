/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once
#include <string>
#include <vector>
#include <functional>
#include <math.h>
#include <unistd.h>

#define MAX_RETRIES 3
#define RETRY_SLEEP 1000000

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
	/// The integer corresponding to string on success, or 0 when the string cannot be converted.
	/// If the string cannot be converted, errno is set to EDOM. If the converted string would fall
	/// out of the range of an integer, errno is set to ERANGE. Strings of the form "123AAA" are parsed
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
	/// Safely attempts to convert a string to a corresponding double.
	/// </summary>
	/// <param name="str">
	/// The string to convert.
	/// </param>
	/// <returns>
	/// double corresponding to string on success, or 0 when the string cannot be converted.
	/// If the string cannot be converted, errno is set to EDOM. If the converted string would fall
	/// out of the range of a long long, errno is set to ERANGE. Strings of the form "123AAA" are
	/// parsed to 123.
	/// </returns>
	static long long safeStod(std::string str);

	/// <summary>
	/// Appends a char vector by a string and adds a special character at the end.
	/// </summary>
	/// <param name="result"> The base char vector. </param>
	/// <param name="word"> The string to be appended to the char vector. </param>
	/// <param name="delimiter"> The character to be added at the end of the vector. </param>
	static void appendBy(std::vector<char> &result, std::string word, char delimiter);

	/// <summary>
	/// Appends a character vector by multiple strings while separating the strings by special characters
	/// and adds a character to the end.
	/// </summary>
	/// <param name="result"> The base char vector. </param>
	/// <param name="words"> The strings to be appended to the vector. </param>
	/// <param name="wordSeparator"> The separator for different words. </param>
	/// <param name="endChar"> The character appended to the vector at the end. </param>
	static void appendBy(std::vector<char> &result, std::vector<std::string> words, char wordSeparator, char endChar);

	/// <summary>
	/// Splits a string on a special character.
	/// </summary>
	/// <param name="str"> The string to be split. </param>
	/// <param name="delimiter"> The character on which the string is split. </param>
	/// <returns> A vector of the substrings obtained by splitting the string. </returns>
	static std::vector<std::string> splitStringOn(std::string str, char delimiter);

	/// <summary>
	/// Changes a string with the format of a hash to a string with the format of a UUID.
	/// </summary>
	/// <param name="hash"> The hash to be converted to a UUID string. </param>
	/// <returns> The hash with format of a UUID string. </returns>
	static std::string hashToUUIDString(std::string hash);

	/// <summary>
	/// Changes a string with the format of a uuid to a string with the format of a hash.
	/// </summary>
	/// <param name="uuid"> The string to be converted to a hash. </param>
	/// <returns> The UUID with format of a hash. </returns>
	static std::string uuidStringToHash(std::string uuid);

	/// <summary>
	/// Gets the current time since epoch in seconds, represented as an integer.
	/// </summary>
	static long long getCurrentTimeSeconds();

	/// <summary>
	/// Gets the current time since epoch in milliseconds, represented as an integer.
	/// </summary>
	static long long getCurrentTimeMilliSeconds();

	/// <summary>
	/// A general template for a query to be performed with retries.
	/// </summary>
	/// <typeparam name="T">
	/// The output of the query. If no output is given, an empty tuple (std::tuple<>) should be used.
	/// </typeparam>
	/// <param name="query"> The corresponding query to be performed a single time. </param>
	/// <returns> Output of the query, which may be an empty tuple (to replace void). </returns>
	template <class T> static T queryWithRetry(std::function<T()> query)
	{
		errno = 0;
		int retries = 0;
		T items;
		do
		{
			items = query();
			if (errno != 0 && errno != ERANGE)
			{
				usleep(pow(2, retries) * RETRY_SLEEP);
				retries++;
			}
		} while (errno != 0 && errno != ERANGE && retries <= MAX_RETRIES);
		if (retries > MAX_RETRIES)
		{
			errno = ENETUNREACH;
		}
		return items;
	};
};
