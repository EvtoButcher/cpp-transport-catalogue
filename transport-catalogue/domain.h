#pragma once
#include <string_view>
#include <string>
#include <vector>

#include "geo.h"

namespace tc_project {

namespace domain {

struct Stop
{
	Stop() = default;
	Stop(std::string_view n, double latit, double longit);

	std::string name;
	geo::Coordinates coordinates;
};

struct Bus
{
	Bus() = default;
	Bus(std::string_view n, std::vector<Stop*> v, bool is_round);

	std::string name;
	std::vector<Stop*> stop_on_route;
	bool is_roundtrip;
};

struct StopInfo
{
	StopInfo() = default;
	StopInfo(std::string_view n, std::vector<Bus*> b);

	std::string_view name;
	std::vector<Bus*> bus_on_route;
};

struct BusInfo
{
	BusInfo() = default;
	BusInfo(size_t s, size_t u_s, double l, double c);

	size_t stops;
	size_t unique_stop;
	double route_length;
	double route_curvature;
};

namespace detail {

	struct PairStopHasher {
		size_t operator() (const std::pair<Stop*, Stop*> stops) const {
			return stop_hasher(stops.first) + stop_hasher(stops.second) * 37;
		}

		std::hash<const void*> stop_hasher;
	};

	struct BusCmp {
		bool operator()(Bus* bus_l, Bus* bus_r) const {
			return bus_r->name > bus_l->name;
		}
	};

}//namespace detail

}//namespace domain

}//namespace tc_project
