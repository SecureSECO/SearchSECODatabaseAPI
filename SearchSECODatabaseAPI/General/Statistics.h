/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
� Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#pragma once

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <queue>

#define SYNCHRONIZE_DELAY 10000000 // 10 seconds.

/// <summary>
/// Enumerates the different possible families.
/// </summary>
enum StatFamily
{
	reqCount,
	methCount,
	langCount,
	jobCount,
	reqTime,
	vulnCount,
	recProj,
	recVuln
};

/// <summary>
/// Stores the Prometheus statistics variables.
/// </summary>
class Statistics
{
  public:
	/// <summary>
	/// Initializes all statistics objects.
	/// </summary>
	virtual void Initialize();

	/// <summary>
	/// Add a recently added project.
	/// </summary>
	/// <param name="url"> The url of the project added.</param>
	void addRecentProject(std::string url);

	/// <summary>
	/// Add a recently added vulnerability.
	/// </summary>
	/// <param name="vulnCode"> The vulnerability code of the vulnerability added.</param>
	void addRecentVulnerability(std::string vulnCode);

	/// <summary>
	/// Regularly synchronizes the statistics to the passed file.
	/// </summary>
	/// <param name="file"> The file to write the statistic to.</param>
	void synchronize(std::string file);

	/// <summary>
	/// Write the value of the statistics to the given file.
	/// </summary>
	/// <param name="file"> The file to write the statistics to.</param>
	void writeToFile(std::string file);

	/// <summary>
	/// Reads the values of the statistics from the passed file.
	/// </summary>
	/// <param name="file"> The file to read the statistics from.</param>
	void readFromFile(std::string file);

	// The families to store the statistics in.
	prometheus::Family<prometheus::Counter> *requestCounter;
	prometheus::Family<prometheus::Counter> *methodCounter;
	prometheus::Family<prometheus::Counter> *languageCounter;
	prometheus::Family<prometheus::Counter> *jobCounter;
	prometheus::Family<prometheus::Gauge> *latestRequest;
	prometheus::Family<prometheus::Counter> *vulnCounter;
	prometheus::Family<prometheus::Gauge> *recentProjects;
	prometheus::Family<prometheus::Gauge> *recentVulns;

	// The ip of this node for identifying the node in the statistics.
	std::string myIP;

	// A boolean to store whether there has been a new request.
	bool newRequest;

  protected:
	// The exposer for the statistics.
	prometheus::Exposer *exposer;

	// The registry that stores the families.
	std::shared_ptr<prometheus::Registry> registry;

	std::queue<prometheus::Gauge *> recentProjectsQueue;
	std::queue<prometheus::Gauge *> recentVulnsQueue;
};