﻿#pragma once
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <utility>
#include <optional>
#include <set>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue {

using namespace domain;

class TransportCatalogue
{
public:

	TransportCatalogue() = default;

	void AddStop(std::string_view name, geo::Coordinates coordinates);
	void AddBus(std::string_view name, const std::vector<std::string_view>& stop_on_route, bool is_roundtrip);
	void AddDistanceFromTo(std::string_view current_stop_name, std::string_view other_stop_name, unsigned int distance_to_stops);

	const Bus* FindBus(std::string_view name) const;
	const Stop* FindStop(std::string_view name) const;

	std::optional<BusInfo> GetBusInfo(std::string_view name) const;
	std::optional <StopInfo> GetStopInfo(std::string_view name) const;
	std::optional<double> GetRoadDistance(std::string_view name) const;
	std::optional< std::vector<domain::Bus*>> GetSortedAllBuses() const;

	bool BusExists(std::string_view name) const;
	bool StopExists(std::string_view name) const;

private:

	std::deque<Stop> list_of_stops_{};//остановок
	std::unordered_map<std::string_view, Stop*> map_of_stops_{};//доступ к остановке по имени за О(1)

	std::deque<Bus> list_of_bus_{};//маршруты
	std::unordered_map<std::string_view, Bus*> map_of_bus_{};//доступ к маршруту по имени за О(1)

	std::unordered_map <Stop*, std::set<Bus*, detail::BusCmp>> map_bus_on_stop_{};//автобусы на остановке

	std::unordered_map<std::pair<Stop*, Stop*>, unsigned int, detail::PairStopHasher> map_distance_between_stops{};//расстояния между остановками 
};

}//namespace transport_catalogue