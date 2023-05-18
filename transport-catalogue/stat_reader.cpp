#include <algorithm>
#include <iostream>
#include <iomanip>
#include <utility>
#include <fstream>

#include "stat_reader.h"

namespace transport_catalogue::file_unloader{

FileUnloader::FileUnloader(TransportCatalogue& catalogue, std::istream& input)
{
	std::string line;
	std::getline(input, line);
	number_of_queryies = static_cast<size_t>(std::stoi(line));

	fresh_queryies_.reserve(number_of_queryies);

	for (; number_of_queryies != 0; --number_of_queryies) {
		std::getline(input, line);
		Query query = ParseQuery(line);

		if (query.key == 'B' && !catalogue.BusExists(query.name)) {
			bus_queryies_.push_back(std::string(query.name));
			fresh_queryies_.push_back(Query(query.key, bus_queryies_.back()));
			continue;
		}

		if (query.key == 'S' && !catalogue.StopExists(query.name)) {
			stop_queryies_.push_back(std::string(query.name));
			fresh_queryies_.push_back(Query(query.key, stop_queryies_.back()));
			continue;
		}

		if (query.key == 'B') {
			if (processed_bus_queries_.find(query.name) != processed_bus_queries_.end()) {
				fresh_queryies_.push_back(Query(query.key, processed_bus_queries_.find(query.name)->first));
			}
			else {
				bus_queryies_.push_back(std::string(query.name));
				fresh_queryies_.push_back(Query(query.key, bus_queryies_.back()));
				processed_bus_queries_[bus_queryies_.back()] = catalogue.GetBusInfo(query.name);
			}
		}
		else {
			if (processed_stop_queries_.find(query.name) != processed_stop_queries_.end()) {
				fresh_queryies_.push_back(Query(query.key, processed_stop_queries_.find(query.name)->first));
			}
			else {
				stop_queryies_.push_back(std::string(query.name));
				fresh_queryies_.push_back(Query(query.key, stop_queryies_.back()));
				processed_stop_queries_[stop_queryies_.back()] = catalogue.GetStopInfo(query.name);
			}
		}
	}
}

Query FileUnloader::ParseQuery(std::string_view line)
{
	char key_query = std::string(line.substr(line.find_first_not_of(' '), line.find_first_not_of(' ') + 1))[0];
	std::string_view name;

	if (key_query == 'B') {
		name = line.substr(line.find_first_not_of(' ', line.find('s') + 1), line.find_last_not_of(' ') + 1);
	}
	else {
		name = line.substr(line.find_first_not_of(' ', line.find('p') + 1), line.find_last_not_of(' ') - line.find_first_not_of(' ', line.find('p') + 1) + 1);
	}

	return Query(key_query, name);
}

void FileUnloader::GetResult(std::ostream& output)
{
	std::for_each(fresh_queryies_.begin(), fresh_queryies_.end(),
		[&](auto& query) {
			std::string answer;
			if (query.key == 'B') {
				if (processed_bus_queries_.find(query.name) == processed_bus_queries_.end()) {
					answer = std::string("Bus ") + std::string(query.name) + std::string(": not found\n");

					output << answer;
				}
				else {
					answer = std::string("Bus ") + std::string(query.name) + std::string(": ") +
						std::to_string(processed_bus_queries_.at(query.name).stops) + std::string(" stops on route, ") +
						std::to_string(processed_bus_queries_.at(query.name).unique_stop) + std::string(" unique stops, ");

					output << answer << std::setprecision(6) << processed_bus_queries_.at(query.name).route_length
						<< std::string(" route length, ") << std::setprecision(6) << processed_bus_queries_.at(query.name).route_curvature
						<< std::string(" curvature\n");
				}
			}
			else {
				if (processed_stop_queries_.find(query.name) == processed_stop_queries_.end()) {
					answer = std::string("Stop ") + std::string(query.name) + std::string(": not found\n");

					output << answer;
				}
				else {
					if (processed_stop_queries_.at(query.name).bus_on_route.empty()) {
						answer = std::string("Stop ") + std::string(query.name) + std::string(": no buses\n");

						output << answer;
					}
					else {

						answer = std::string("Stop ") + std::string(query.name) + std::string(":") + std::string(" buses ");

						for (const auto& bus : processed_stop_queries_.at(query.name).bus_on_route) {
							answer += bus->name + std::string(" ");
						}
						answer.pop_back();
						answer += std::string("\n");

						output << answer;
					}
				}
			}
		});
}

}//namespace transport_catalogue::file_unloader
