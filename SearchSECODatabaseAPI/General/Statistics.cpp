/*
This program has been developed by students from the bachelor Computer Science at
Utrecht University within the Software Project course.
© Copyright Utrecht University (Department of Information and Computing Sciences)
*/

#include "Statistics.h"
#include "Utility.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

void Statistics::Initialize()
{
	// Create an http server running on port 8004.
	exposer = new prometheus::Exposer("8004");

	// Create a metrics registry.
	registry = std::make_shared<prometheus::Registry>();

	// Create the counter and gauge families.
	requestCounter =
		&prometheus::BuildCounter().Name("api_requests_total").Help("Number of observed requests.").Register(*registry);

	methodCounter =
		&prometheus::BuildCounter().Name("api_methods_total").Help("Number of received methods.").Register(*registry);

	languageCounter = &prometheus::BuildCounter()
						   .Name("api_languages_byte_total")
						   .Help("Bytes of languages encountered.")
						   .Register(*registry);

	jobCounter = &prometheus::BuildCounter()
					  .Name("api_finished_jobs_total")
					  .Help("Number of finished jobs.")
					  .Register(*registry);

	latestRequest = &prometheus::BuildGauge()
						 .Name("api_request_time_seconds")
						 .Help("The latest time a request has been received.")
						 .Register(*registry);

	vulnCounter = &prometheus::BuildCounter()
					   .Name("api_vulnerabilities_total")
					   .Help("Number of vulnerabilities.")
					   .Register(*registry);

	// Ask the exposer to scrape the registry on incoming HTTP requests.
	exposer->RegisterCollectable(registry);
}

void Statistics::synchronize(std::string file)
{
	while (true)
	{
		usleep(SYNCHRONIZE_DELAY);
		if (newRequest)
		{
			writeToFile(file);
			newRequest = false;
		}
	}
}

void Statistics::readFromFile(std::string file)
{
	std::ifstream fileHandler;
	fileHandler.open(file);

	if (!fileHandler.is_open())
	{
		std::cout << "Unable to open statistics file." << std::endl;
		return;
	}
	std::string line;
	StatFamily current;
	while (std::getline(fileHandler, line))
	{
		if (line[0] == '#')
		{
			if (line == "#requestCount")
			{
				current = reqCount;
			}
			else if (line == "#methodCount")
			{
				current = methCount;
			}
			else if (line == "#jobCount")
			{
				current = jobCount;
			}
			else if (line == "#languageCount")
			{
				current = langCount;
			}
			else if (line == "#requestTime")
			{
				current = reqTime;
			}
			else if (line == "#vulnerabilityCount")
			{
				current = vulnCount;
			}
			continue;
		}
		auto lineSplitted = Utility::splitStringOn(line, '?');
		if (lineSplitted.size() >= 3)
		{
			switch (current)
			{
			case reqCount:
				requestCounter
					->Add({{"Node", lineSplitted[1]}, {"Client", lineSplitted[0]}, {"Request", lineSplitted[2]}})
					.Increment(Utility::safeStod(lineSplitted[3]));
				break;
			case methCount:
				methodCounter
					->Add({{"Node", lineSplitted[2]}, {"Client", lineSplitted[0]}, {"Extension", lineSplitted[1]}})
					.Increment(Utility::safeStod(lineSplitted[3]));
				break;
			case langCount:
				languageCounter
					->Add({{"Node", lineSplitted[2]}, {"Client", lineSplitted[0]}, {"Language", lineSplitted[1]}})
					.Increment(Utility::safeStod(lineSplitted[3]));
				break;
			case jobCount:
				jobCounter
					->Add({{"Node", lineSplitted[1]}, {"Client", lineSplitted[0]}, {"Reason", lineSplitted[2]}})
					.Increment(Utility::safeStod(lineSplitted[3]));
				break;
			case reqTime:
				latestRequest
					->Add({{"Node", lineSplitted[1]}, {"Client", lineSplitted[0]}, {"Request", lineSplitted[2]}})
					.Set(Utility::safeStod(lineSplitted[3]));
				break;
			case vulnCount:
				vulnCounter->Add({{"Node", lineSplitted[1]}, {"Client", lineSplitted[0]}})
					.Increment(Utility::safeStod(lineSplitted[2]));
				break;
			default:
				break;
			}
		}
	}
	fileHandler.close();
}

void Statistics::writeToFile(std::string file)
{
	std::stringstream fileData;
	
	fileData << "#requestCount\n";

	std::vector<prometheus::MetricFamily> family = requestCounter->Collect();

	if (family.size() >= 1)
	{
		for (prometheus::ClientMetric metric : family[0].metric)
		{
			fileData << metric.label[0].value << "?" << metric.label[1].value << "?" << metric.label[2].value << "?"
						<< std::to_string(metric.counter.value) << "\n";
		}
	}

	fileData << "#methodCount\n";

	family = methodCounter->Collect();

	if (family.size() >= 1)
	{
		for (prometheus::ClientMetric metric : family[0].metric)
		{
			fileData << metric.label[0].value << "?" << metric.label[1].value << "?" << metric.label[2].value << "?"
						<< std::to_string(metric.counter.value) << "\n";
		}
	}

	fileData << "#jobCount\n";

	family = jobCounter->Collect();

	if (family.size() >= 1)
	{
		for (prometheus::ClientMetric metric : family[0].metric)
		{
			fileData << metric.label[0].value << "?" << metric.label[1].value << "?" << metric.label[2].value << "?"
						<< std::to_string(metric.counter.value) << "\n";
		}
	}

	fileData << "#languageCount\n";

	family = languageCounter->Collect();

	if (family.size() >= 1)
	{
		for (prometheus::ClientMetric metric : family[0].metric)
		{
			fileData << metric.label[0].value << "?" << metric.label[1].value << "?" << metric.label[2].value << "?"
						<< std::to_string(metric.counter.value) << "\n";
		}
	}

	fileData << "#requestTime\n";

	family = latestRequest->Collect();

	if (family.size() >= 1)
	{
		for (prometheus::ClientMetric metric : family[0].metric)
		{
			fileData << metric.label[0].value << "?" << metric.label[1].value << "?" << metric.label[2].value << "?"
						<< std::to_string(metric.gauge.value) << "\n";
		}
	}

	fileData << "#vulnerabilityCount\n";

	family = vulnCounter->Collect();

	if (family.size() >= 1)
	{
		for (prometheus::ClientMetric metric : family[0].metric)
		{
			fileData << metric.label[0].value << "?" << metric.label[1].value << "?"
					 << std::to_string(metric.counter.value) << "\n";
		}
	}

	std::ofstream fileHandler;
	fileHandler.open(file);

	if (!fileHandler.is_open())
	{
		std::cout << "Unable to open statistics file." << std::endl;
		return;
	}

	fileHandler << fileData.str();

	fileHandler.close();
}