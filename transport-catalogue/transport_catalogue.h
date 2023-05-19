#pragma once
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
#include "input_reader.h"

namespace transport_catalogue {

struct Stop
{
	Stop() = default;

	Stop(std::string_view n, double latit, double longit)
		: name(std::string(n))
		, coordinates(latit, longit)
	{};

	std::string name;

	Coordinates coordinates;
};

struct Bus
{
	Bus() = default;

	Bus(std::string_view n, std::vector<Stop*> v)
		: name(std::string(n))
		, stop_on_route(v)
	{};

	std::string name;
	std::vector<Stop*> stop_on_route;
};

struct StopInfo
{
	StopInfo() = default;

	StopInfo(std::string_view n, std::vector<Bus*> b)
		:name(n),
		bus_on_route(std::move(b))
	{};

	std::string_view name;
	std::vector<Bus*> bus_on_route;
};

struct BusInfo
{
	BusInfo() = default;

	BusInfo(size_t s, size_t u_s, double l, double c)
		: stops(s)
		, unique_stop(u_s)
		, route_length(l)
		, route_curvature(c)
	{};

	size_t stops;
	size_t unique_stop;
	double route_length;
	double route_curvature;
};

struct PairStopHasher {
	size_t operator() (const std::pair<Stop*, Stop*> stops) const {
		return stop_hasher(stops.first) + stop_hasher(stops.second) * 37;
	}

	std::hash<const void*> stop_hasher;
};

struct BusCmp {
	bool operator()(Bus* bus_l, Bus* bus_r) const{
		return bus_r->name > bus_l->name;
	}
};

class TransportCatalogue
{
public:

	TransportCatalogue() = default;

	void AddStop(std::string_view name, Coordinates coordinates);
	void AddBus(std::string_view name, const std::vector<std::string_view>& stop_on_route);
	void AddDistanceFromTo(std::string_view current_stop_name, std::string_view other_stop_name, unsigned int distance_to_stops);

	const Bus* FindBus(std::string_view name) const;
	const Stop* FindStop(std::string_view name) const;

	std::optional<BusInfo> GetBusInfo(std::string_view name) const;
	std::optional <StopInfo> GetStopInfo(std::string_view name) const;
	std::optional<double> GetRoadDistance(std::string_view name) const;

	bool BusExists(std::string_view name) const;
	bool StopExists(std::string_view name) const;

private:

	std::deque<Stop> list_of_stops_{};//остановок
	std::unordered_map<std::string_view, Stop*> map_of_stops_{};//доступ к остановке по имени за О(1)

	std::deque<Bus> list_of_bus_{};//маршруты
	std::unordered_map<std::string_view, Bus*> map_of_bus_{};//доступ к маршруту по имени за О(1)

	std::unordered_map <Stop*, std::set<Bus*, BusCmp>> map_bus_on_stop_{};//автобусы на остановке

	std::unordered_map<std::pair<Stop*, Stop*>, unsigned int, PairStopHasher> map_distance_between_stops{};//расстояния между остановками 
};

}//namespace transport_catalogue