/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "DatabaseRequestHandler.h"
#include "Definitions.h"
#include "HTTPStatus.h"
#include "Utility.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <regex>
#include <thread>
#include <future>
#include <utility>

template <class T> T DatabaseRequestHandler::queryWithRetry(std::function<T()> function)
{
	errno = 0;
	int retries = 0;
	T items;
	do
	{
		items = function();
		retries++;
	} while (errno != 0 && errno != ERANGE && retries <= MAX_RETRIES);
	if (retries > MAX_RETRIES)
	{
		errno = ENETUNREACH;
		// return NULL;
	}
	return items;
}

template <class T> std::vector<std::vector<T>> DatabaseRequestHandler::toChunks(std::vector<T> list, int chunkSize)
{
	std::vector<T> currentChunk = {};
	std::vector<std::vector<T>> chunks = {};
	for (T element : list)
	{
		currentChunk.push_back(element);
		if (currentChunk.size() >= chunkSize)
		{
			chunks.push_back(currentChunk);
			currentChunk.clear();
		}
	}
	if (currentChunk.size() > 0)
	{
		chunks.push_back(currentChunk);
	}

	return chunks;
}

template <class T1, class T2>
std::queue<std::pair<std::vector<T1>, std::vector<T2>>>
DatabaseRequestHandler::cartesianProductQueue(std::vector<std::vector<T1>> listT1, std::vector<std::vector<T2>> listT2)
{
	std::queue<std::pair<std::vector<T1>, std::vector<T2>>> pairQueue;
	for (std::vector<T1> elemT1 : listT1)
	{
		for (std::vector<T2> elemT2 : listT2)
		{
			pairQueue.push(std::make_pair(elemT1, elemT2));
		}
	}

	return pairQueue;
}