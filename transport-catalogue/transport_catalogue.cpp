#include <algorithm>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "log_duration.h"

using namespace transport_catalogue::detail;

namespace transport_catalogue {

void TransportCatalogue::AddStop(const std::string_view name, const double latitude, const double longitude)
{
	if (StopExists(name)) {
		return;
	}

	list_of_stops_.emplace_back(std::string(name), latitude, longitude);
	map_of_stops_[list_of_stops_.back().name] = &list_of_stops_.back();
}

void TransportCatalogue::AddBus(const std::string_view name, const std::vector<Stop*>& stop_on_route)
{
	if (BusExists(name)) {
		return;
	}

	list_of_bus_.emplace_back(Bus(std::string(name), stop_on_route));
	map_of_bus_[list_of_bus_.back().name] = &list_of_bus_.back();
}

void TransportCatalogue::AddBus(const std::string_view name, const std::vector<std::string_view>& stop_on_route)
{
	if (BusExists(name)) {
		return;
	}

	std::vector<Stop*> st;
	st.reserve(stop_on_route.size());
	std::for_each(stop_on_route.begin(), stop_on_route.end(),
		[&](const auto& stop_name) {
			st.push_back(map_of_stops_.at(stop_name));
		});
	AddBus(name, std::move(st));
}

void TransportCatalogue::AddStopInfo()
{
	for (auto& [_, stop_ptr] : map_of_stops_) {
		std::vector<Bus*> bus_tmp;
		for (auto& [_, bus_ptr] : map_of_bus_) {
			for (auto stop_on_route_ptr : bus_ptr->stop_on_route) {
				if (stop_ptr == stop_on_route_ptr) {
					bus_tmp.push_back(bus_ptr);
				}
			}
		}

		std::sort(bus_tmp.begin(), bus_tmp.end(),
			[&](Bus* l_bus, Bus* r_bus) {
				return r_bus->name > l_bus->name;
			});
		bus_tmp.erase(std::unique(bus_tmp.begin(), bus_tmp.end()), bus_tmp.end());

		map_bus_on_stop_[stop_ptr] = std::move(bus_tmp);
	}
}

void TransportCatalogue::AddDistanceFromTo(const std::string_view stop_name, const std::vector<std::pair<std::string_view, unsigned int>>& distance_to_stops)
{
	for (const auto& [other_stop_name, dist] : distance_to_stops) {
		map_distance_between_stops[std::make_pair(map_of_stops_.at(stop_name), map_of_stops_.at(other_stop_name))] = dist;

		if (map_distance_between_stops.find(std::make_pair(map_of_stops_.at(other_stop_name), map_of_stops_.at(stop_name))) == map_distance_between_stops.end()) {
			map_distance_between_stops[std::make_pair(map_of_stops_.at(other_stop_name), map_of_stops_.at(stop_name))] = dist;
		}
	}
}

Bus TransportCatalogue::FindBus(const std::string_view name)
{
	return *map_of_bus_[name];
}

Stop TransportCatalogue::FindStop(const std::string_view name)
{
	return *map_of_stops_[name];
}

BusInfo TransportCatalogue::GetBusInfo(const std::string_view name)
{
	double real_lenght = GetRoadDistance(name);
	double route_curvature = 0;

	Bus bus = FindBus(name);
	std::vector<std::string_view> unique_stops_tmp;
	unique_stops_tmp.reserve(bus.stop_on_route.size());

	for (int i = 0; i < bus.stop_on_route.size(); ++i) {
		if (i + 1 < bus.stop_on_route.size()) {
			route_curvature += detail::ComputeDistance(bus.stop_on_route[i]->coordinates, bus.stop_on_route[i + 1]->coordinates);
		}
		unique_stops_tmp.push_back(bus.stop_on_route[i]->name);
	}

	//узкое место 
	std::sort(unique_stops_tmp.begin(), unique_stops_tmp.end());
	unique_stops_tmp.erase(std::unique(unique_stops_tmp.begin(), unique_stops_tmp.end()), unique_stops_tmp.end());

	return { bus.stop_on_route.size(), unique_stops_tmp.size(), real_lenght, (real_lenght / route_curvature) };
}

StopInfo TransportCatalogue::GetStopInfo(const std::string_view name)
{
	return StopInfo(map_of_stops_.at(name)->name, map_bus_on_stop_.at(map_of_stops_.at(name)));
}

double TransportCatalogue::GetRoadDistance(const std::string_view bus_name)
{
	double real_distance = 0.0;
	auto& route = map_of_bus_.at(bus_name)->stop_on_route;
	for (int i = 0; i + 1 < route.size(); ++i) {
		real_distance += map_distance_between_stops[std::make_pair(route[i], route[i + 1])];
	}

	return real_distance;
}

bool TransportCatalogue::BusExists(std::string_view name) const
{
	return map_of_bus_.find(name) != map_of_bus_.end();
}

bool TransportCatalogue::StopExists(std::string_view name) const 
{
	return map_of_stops_.find(name) != map_of_stops_.end();
}


void StartCatalogue(std::istream& input, std::ostream& output)
{
	file_loader::FileLoader file(input);
	TransportCatalogue catalogue;

	for (const auto& [name, stop] : file.GetBusStop()) {
		catalogue.AddStop(name, std::get<0>(stop), std::get<1>(stop));
	}

	for (const auto& [name, route] : file.GetBusRoute()) {
		catalogue.AddBus(name, route);
	}

	catalogue.AddStopInfo();

	for (auto& [name, distance_to] : file.GetDistances()) {
		catalogue.AddDistanceFromTo(name, distance_to);
	}

	file_unloader::FileUnloader query(catalogue, input);

	query.GetResult(output);
}

}//namespace transport_catalogue