#include <algorithm>
#include <numeric>

#include "input_reader.h"

namespace transport_catalogue::file_loader{

FileLoader::FileLoader(std::istream& input)
{
	std::string line;
	std::getline(input, line);
	size_t number_of_additions_ = static_cast<size_t>(std::stoi(line));

	for (; number_of_additions_ != 0; --number_of_additions_) {
		std::getline(input, line);
		ParseQuery(line);
	}
}

void FileLoader::ParseQuery(std::string_view line)
{
	std::string_view key = line.substr(line.find_first_not_of(' '), line.find_first_of(' ', line.find_first_not_of(' ')));

	if (key == std::string_view("Stop")) {
		auto [name, geo] = ParseStop(line);
		if (bus_stop_.find(name) == bus_stop_.end()) {
			name_ini.emplace_back(std::string(name));
			bus_stop_[name_ini.back()];
		}
		bus_stop_[name] = std::make_tuple(std::get<0>(geo), std::get<1>(geo));

		auto distance_from_current_stop_to_stops = std::move(std::get<2>(geo));
		distance_to_stops_[bus_stop_.find(name)->first] = std::vector<std::pair<std::string_view, unsigned int>>(distance_from_current_stop_to_stops.size());
		std::transform(distance_from_current_stop_to_stops.begin(), distance_from_current_stop_to_stops.end()
			, distance_to_stops_[bus_stop_.find(name)->first].begin()
			, [&](const auto& neme_distance) {
				if (bus_stop_.find(neme_distance.first) == bus_stop_.end()) {
					name_ini.emplace_back(std::string(neme_distance.first));
					bus_stop_[name_ini.back()];
				}
				return std::make_pair(bus_stop_.find(neme_distance.first)->first, neme_distance.second);
			});

		return;
	}

	if (key == std::string_view("Bus")) {
		auto [name, route] = ParseBus(line);
		name_ini.emplace_back(std::string(name));

		std::vector<std::string> buffer;//служит для инициализации bus_ini из route
		buffer.reserve(route.size());
		std::for_each(route.begin(), route.end(),
			[&](auto stop) {
				buffer.push_back(std::string(stop));
			});
		bus_ini.emplace_back(buffer);

		route_[name_ini.back()] = { bus_ini.back().begin(), bus_ini.back().end() };
		return;
	}
}

std::pair<std::string_view, std::tuple<double, double, std::vector<std::pair<std::string_view, double>>>> ParseStop(std::string_view& line)
{
	auto centr_pos = line.find(':');
	auto stop_pos = line.find_first_of('p');
	std::string_view name = line.substr(line.find_first_not_of(' ', stop_pos + 1)
		, centr_pos - line.find_first_not_of('p', stop_pos) - 1);// название остановки 

	auto new_centr = line.find(',');
	std::string_view  latitude = line.substr(line.find_first_not_of(' ', centr_pos + 1)
		, new_centr - line.find_first_not_of(' ', centr_pos + 1));//широта;

	std::string_view longitude;
	std::vector<std::pair<std::string_view, double>> distance_from_current_stop_to_stops;// расстояние от текущей остановки до слудующий остановки 
	if (line.find(',', new_centr + 1) < line.size()) {
		longitude = line.substr(line.find_first_not_of(' ', new_centr + 1)
			, (line.find(',', new_centr + 1) - line.find_first_not_of(' ', new_centr + 1)));//долгота;

		new_centr = line.find(',', new_centr + 1);

		double distance = 0;
		std::string_view to_stop_name;
		for (; new_centr < line.size(); new_centr = line.find(',', new_centr + 1)) {

			distance = std::stod(std::string(line.substr(line.find_first_not_of(' ', new_centr + 1),
				(line.find_first_of('m', new_centr) - new_centr) - 2)));

			to_stop_name = line.substr(line.find_first_not_of(' ', line.find_first_of('o', line.find('m', new_centr + 1)) + 1)
				, line.find(',', new_centr + 1) - line.find_first_not_of(' ', line.find_first_of('o', line.find('m', new_centr + 1)) + 1));

			distance_from_current_stop_to_stops.push_back(std::make_pair(to_stop_name, distance));
		}
	}
	else {
		longitude = line.substr(line.find_first_not_of(' ', new_centr + 1)
			, (line.find_first_of(' ', line.find_first_not_of(' ', new_centr + 1)) - line.find_first_not_of(' ', new_centr + 1)));//долгота;
	}

	return std::make_pair(name, std::make_tuple(std::stod(std::string(latitude)), stod(std::string(longitude)), distance_from_current_stop_to_stops));
}

std::pair<std::string_view, std::vector<std::string_view>> ParseBus(std::string_view& line)
{
	auto stop_pos = line.find_first_of('s');
	std::string_view name = line.substr(line.find_first_not_of(' ', stop_pos + 1)
		, line.find(':') - line.find_first_not_of('s', stop_pos) - 1);// название маршрута

	//определение какого типа добавляем маршрут(кольцевой/обычный)
	char search_car;
	bool is_circle_route = true;
	if (line.find('>') <= line.size()) {
		search_car = '>';
	}
	else {
		search_car = '-';
		is_circle_route = false;
	}

	//парсинг остановок на маршруте
	std::vector<std::string_view> stop_on_rout;
	auto centr_pos = line.find(':');
	for (auto new_centr = line.find(search_car); centr_pos < line.size(); new_centr = line.find(search_car, new_centr + 1)) {
		std::string_view  stop = line.substr(line.find_first_not_of(' ', centr_pos + 1)
			, new_centr - (centr_pos + 3));
		centr_pos = new_centr;

		stop_on_rout.push_back(stop);
	}

	//дополнение обратного пути для не кольцевого маршрута
	if (!is_circle_route) {
		std::vector<std::string_view> route_to_end(stop_on_rout.rbegin(), stop_on_rout.rend());
		route_to_end.erase(route_to_end.begin());
		stop_on_rout.reserve(stop_on_rout.size() * 2);
		std::move(route_to_end.begin(), route_to_end.end(), std::back_inserter(stop_on_rout));
	}

	return std::make_pair(name, stop_on_rout);
}


std::unordered_map<std::string_view, std::tuple<double, double>> FileLoader::GetBusStop()
{
	return bus_stop_;
}

std::unordered_map<std::string_view, std::vector<std::string_view>> FileLoader::GetBusRoute()
{
	return route_;
}

std::unordered_map<std::string_view, std::vector<std::pair<std::string_view, unsigned int>>> FileLoader::GetDistances()
{
	return distance_to_stops_;
}

}//namespace transport_catalogue::file_loader