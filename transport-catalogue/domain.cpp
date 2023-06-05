#include "domain.h"

namespace tc_project {

namespace domain {

BusInfo::BusInfo(size_t s, size_t u_s, double l, double c)
	: stops(s)
	, unique_stop(u_s)
	, route_length(l)
	, route_curvature(c)
{}

StopInfo::StopInfo(std::string_view n, std::vector<Bus*> b)
	:name(n),
	bus_on_route(std::move(b))
{}

Bus::Bus(std::string_view n, std::vector<Stop*> v, bool is_round)
	: name(std::string(n))
	, stop_on_route(v)
	, is_roundtrip(is_round)
{}

Stop::Stop(std::string_view n, double latit, double longit)

	: name(std::string(n))
	, coordinates(latit, longit)
{}

}//namespace domain

}//namespace tc_project
