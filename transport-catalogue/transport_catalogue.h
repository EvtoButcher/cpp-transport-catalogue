#pragma once
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <utility>

#include "geo.h"
#include "input_reader.h"

namespace transport_catalogue {

namespace detail{
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

}//namespase detail

class TransportCatalogue
{
public:

	TransportCatalogue() = default;
	//TransportCatalogue(std::istream& input);

	void AddStop(const std::string_view name, const double latitude, const double longitude);
	void AddBus(const std::string_view name, const std::vector<detail::Stop*>& stop_on_route);
	void AddBus(const std::string_view name, const std::vector<std::string_view>& stop_on_route);
	void AddStopInfo();
	void AddDistanceFromTo(const std::string_view stop_name, const std::vector<std::pair<std::string_view, unsigned int>>& distance_to_stops);

	detail::Bus FindBus(const std::string_view name);
	detail::Stop FindStop(const std::string_view name);

	detail::BusInfo GetBusInfo(const std::string_view name);
	detail::StopInfo GetStopInfo(const std::string_view name);
	double GetRoadDistance( const std::string_view name);

	bool BusExists(std::string_view name) const;
	bool StopExists(std::string_view name) const;

private:

	std::deque<detail::Stop> list_of_stops_{};//остановок
	std::unordered_map<std::string_view, detail::Stop*> map_of_stops_{};//доступ к остановке по имени за О(1)

	std::deque<detail::Bus> list_of_bus_{};//маршруты
	std::unordered_map<std::string_view, detail::Bus*> map_of_bus_{};//доступ к маршруту по имени за О(1)

	std::unordered_map <detail::Stop*, std::vector<detail::Bus*>> map_bus_on_stop_{};//автобусы на остановке

	std::unordered_map<std::pair<detail::Stop*, detail::Stop*>, unsigned int, detail::PairStopHasher> map_distance_between_stops{};//расстояния между остановками 
};

void StartCatalogue(std::istream& input, std::ostream& output);

}//namespace transport_catalogue