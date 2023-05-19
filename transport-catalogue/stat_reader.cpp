#include <algorithm>
#include <iostream>
#include <iomanip>
#include <utility>
#include <fstream>

#include "stat_reader.h"

namespace transport_catalogue::file_unloader{

FileUnloader::FileUnloader(const TransportCatalogue& catalogue, std::istream& input)
{
	std::string line;
	std::getline(input, line);
	number_of_queryies = static_cast<size_t>(std::stoi(line));

	fresh_queryies_.reserve(number_of_queryies);

	for (; number_of_queryies != 0; --number_of_queryies) {
		std::getline(input, line);
		Query query = ParseQuery(line);

		if (query.key == 'B') {
			if (processed_bus_queries_.find(query.name) != processed_bus_queries_.end()) {
				fresh_queryies_.push_back(Query(query.key, processed_bus_queries_.find(query.name)->first));
			}
			else {
				auto bus_info = catalogue.GetBusInfo(query.name);
				bus_queryies_.push_back(std::string(query.name));
				fresh_queryies_.push_back(Query(query.key, bus_queryies_.back()));
				if (bus_info) { 
					processed_bus_queries_[bus_queryies_.back()] = std::move(*bus_info);
				}
			}
		}
		else {
			if (processed_stop_queries_.find(query.name) != processed_stop_queries_.end()) {
				fresh_queryies_.push_back(Query(query.key, processed_stop_queries_.find(query.name)->first));
			}
			else {
				auto stop_info = catalogue.GetStopInfo(query.name);
				stop_queryies_.push_back(std::string(query.name));
				fresh_queryies_.push_back(Query(query.key, stop_queryies_.back()));
				if (stop_info) {
					processed_stop_queries_[stop_queryies_.back()] = std::move(*stop_info);
				}
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

std::string FileUnloader::GetAnswerToBusRequest(std::string_view name)
{
	std::string bus_answer;
	bus_answer = std::string("Bus ") + std::string(name) + std::string(": ") +
		std::to_string(processed_bus_queries_.at(name).stops) + std::string(" stops on route, ") +
		std::to_string(processed_bus_queries_.at(name).unique_stop) + std::string(" unique stops, ");
	return bus_answer;
}

std::string FileUnloader::GetAnswerToStopRequest(std::string_view name)
{
	std::string stop_answer;
	if (processed_stop_queries_.at(name).bus_on_route.empty()) {
		stop_answer = std::string("Stop ") + std::string(name) + std::string(": no buses\n");
	}
	else {
		stop_answer = std::string("Stop ") + std::string(name) + std::string(":") + std::string(" buses ");

		for (const auto& bus : processed_stop_queries_.at(name).bus_on_route) {
			stop_answer += bus->name + std::string(" ");
		}
		stop_answer.pop_back();
		stop_answer += std::string("\n");
	}
	return stop_answer;
}

void FileUnloader::DisplayResult(std::ostream& output)
{
	for (auto& query : fresh_queryies_) {
		if (query.key == 'B') {
			if (processed_bus_queries_.find(query.name) == processed_bus_queries_.end()) {
				output << std::string("Bus ") + std::string(query.name) + std::string(": not found\n");
			}
			else {
				output << GetAnswerToBusRequest(query.name) << std::setprecision(6) << processed_bus_queries_.at(query.name).route_length
					<< std::string(" route length, ") << std::setprecision(6) << processed_bus_queries_.at(query.name).route_curvature
					<< std::string(" curvature\n");
			}
		}
		else {
			if (processed_stop_queries_.find(query.name) == processed_stop_queries_.end()) {
				output << std::string("Stop ") + std::string(query.name) + std::string(": not found\n");
			}
			else {
				output << GetAnswerToStopRequest(query.name);
			}
		}
	}

}

}//namespace transport_catalogue::file_unloader
