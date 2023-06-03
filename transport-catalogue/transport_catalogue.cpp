#include <algorithm>

#include "transport_catalogue.h"
#include "log_duration.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(const std::string_view name, const geo::Coordinates coordinates)
{
	if (StopExists(name)) {
		return;
	}

	list_of_stops_.emplace_back(std::string(name), coordinates.lat, coordinates.lng);
	map_of_stops_[list_of_stops_.back().name] = &list_of_stops_.back();
}


void TransportCatalogue::AddBus(const std::string_view bus_name, const std::vector<std::string_view>& stop_on_route, bool is_roundtrip)
{
	if (BusExists(bus_name)) {
		return;
	}

	std::vector<Stop*> st;
	st.reserve(stop_on_route.size());
	std::for_each(stop_on_route.begin(), stop_on_route.end(),
		[&](const auto& stop_name) {
			st.push_back(map_of_stops_.at(stop_name));
		});
	
	list_of_bus_.emplace_back(Bus(bus_name, st, is_roundtrip));
	map_of_bus_[list_of_bus_.back().name] = &list_of_bus_.back();
		
	std::for_each(st.begin(), st.end(),
		[&](Stop* stop_ptr) {
			map_bus_on_stop_[stop_ptr].insert(map_of_bus_.at(bus_name));
		});
}

void TransportCatalogue::AddDistanceFromTo(const std::string_view current_stop_name, const std::string_view other_stop_name, const unsigned int distance_to_stops)
{
	map_distance_between_stops[std::make_pair(map_of_stops_.at(current_stop_name), map_of_stops_.at(other_stop_name))] = distance_to_stops;

	if (map_distance_between_stops.find(std::make_pair(map_of_stops_.at(other_stop_name), map_of_stops_.at(current_stop_name))) == map_distance_between_stops.end()) {
		map_distance_between_stops[std::make_pair(map_of_stops_.at(other_stop_name), map_of_stops_.at(current_stop_name))] = distance_to_stops;
	}
}

const Bus* TransportCatalogue::FindBus(const std::string_view name) const
{
	return map_of_bus_.find(name) != map_of_bus_.end() ? map_of_bus_.at(name) : nullptr;
}

const Stop* TransportCatalogue::FindStop(const std::string_view name) const
{
	return map_of_stops_.find(name) != map_of_stops_.end() ? map_of_stops_.at(name) : nullptr;
}

std::optional<BusInfo>  TransportCatalogue::GetBusInfo(const std::string_view name) const
{
	auto bus_ptr = FindBus(name);

	if (!bus_ptr) {
		return std::nullopt;
	}

	double real_lenght = *GetRoadDistance(name);
	double route_curvature = 0;
	
	std::vector<std::string_view> unique_stops_tmp;
	unique_stops_tmp.reserve(bus_ptr->stop_on_route.size());

	for (size_t i = 0; i < bus_ptr->stop_on_route.size(); ++i) {
		if (i + 1 < bus_ptr->stop_on_route.size()) {
			route_curvature += geo::ComputeDistance(bus_ptr->stop_on_route[i]->coordinates, bus_ptr->stop_on_route[i + 1]->coordinates);
		}
		unique_stops_tmp.push_back(bus_ptr->stop_on_route[i]->name);
	}

	//узкое место 
	std::sort(unique_stops_tmp.begin(), unique_stops_tmp.end());
	unique_stops_tmp.erase(std::unique(unique_stops_tmp.begin(), unique_stops_tmp.end()), unique_stops_tmp.end());

	return BusInfo(bus_ptr->stop_on_route.size(), unique_stops_tmp.size(), real_lenght, (real_lenght / route_curvature));
}

std::optional <StopInfo> TransportCatalogue::GetStopInfo(const std::string_view name) const
{
	if (!StopExists(name)) {
		return std::nullopt;
	}

	std::vector<Bus*> buses_tmp;
	if (map_bus_on_stop_.find(map_of_stops_.at(name)) != map_bus_on_stop_.end()) {
		buses_tmp.reserve(map_bus_on_stop_.at(map_of_stops_.at(name)).size());
		buses_tmp.assign(map_bus_on_stop_.at(map_of_stops_.at(name)).begin(), map_bus_on_stop_.at(map_of_stops_.at(name)).end());
	}

	return StopInfo(map_of_stops_.at(name)->name, buses_tmp);
}

std::optional<double> TransportCatalogue::GetRoadDistance(const std::string_view bus_name) const
{
	double real_distance = 0.0;
	auto& route = map_of_bus_.at(bus_name)->stop_on_route;
	for (size_t i = 0; i + 1 < route.size(); ++i) {
		real_distance += map_distance_between_stops.at(std::make_pair(route[i], route[i + 1]));
	}

	return real_distance;
}

std::optional<std::vector<Bus*>> TransportCatalogue::GetSortedAllBuses() const
{
	std::vector<Bus*> all_buses; 
	all_buses.reserve(map_of_bus_.size());

	std::transform(map_of_bus_.cbegin(), map_of_bus_.cend(), std::back_inserter(all_buses),
		[](const auto& key_bus){
			return key_bus.second;
		});

	std::sort(all_buses.begin(), all_buses.end(),
		[](const auto& lhs, const auto& rhs){
			return lhs->name < rhs->name;
		});

	return all_buses;
}

bool TransportCatalogue::BusExists(std::string_view name) const
{
	return map_of_bus_.find(name) != map_of_bus_.end();
}

bool TransportCatalogue::StopExists(std::string_view name) const 
{
	return map_of_stops_.find(name) != map_of_stops_.end();
}

}//namespace transport_catalogue