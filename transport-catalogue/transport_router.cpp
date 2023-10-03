#include "transport_router.h"
#include <stdexcept>

namespace tc_project {

namespace transport_router {

using namespace std::literals;


void TransportRouter::AddRouterSetting(RouterSettings settings) {
	if (!CheckArgument(settings.bus_wait_time_) && !CheckArgument(settings.bus_velocity_)) {
		throw std::invalid_argument("Incorrect wait time or velocity"s);
	}
	settings_ = { settings.bus_wait_time_, settings.bus_velocity_ };
}

bool TransportRouter::CheckArgument(double arg) {
	return arg > 0 && arg <= 1000;
}

void TransportRouter::BuildGraph(graph::DirectedWeightedGraph<RouteWeight>& graph, const transport_catalogue::TransportCatalogue& catalogue_,
	const std::vector<domain::Stop*>& stops, const std::string_view bus_name) {
	for (int i = 0; i < stops.size() - 1; ++i) {
		// суммарное время движения по ребру с учетом ожидания автобуса в минутах
		double route_time = settings_.bus_wait_time_;
		const auto& stop_from = stops[i];
		int span_count = 1;
		for (int j = i + 1; j < stops.size(); ++j) {
			const auto& stop_to = stops[j];
			route_time += ComputeRouteTime(catalogue_, stops[j - 1], stop_to);
			graph.AddEdge({ stopname_id_[stop_from->name], stopname_id_[stop_to->name], {bus_name, route_time, span_count++ } });
		}
	}
}

void TransportRouter::InicializeGraph(const transport_catalogue::TransportCatalogue& catalogue) {
	graph::DirectedWeightedGraph<RouteWeight> graph(CountStops(catalogue));
	for (const auto& [bus_name, route] : catalogue.GetAllBuses()) {
		BuildGraph(graph, catalogue, route->stop_on_route, bus_name);
		if (!route->is_roundtrip) {
			std::vector<domain::Stop*> rstops{ route->stop_on_route.rbegin(), route->stop_on_route.rend() };
			BuildGraph(graph, catalogue, rstops, bus_name);
		}
	}
	graph_ = std::move(graph);
	router_ = std::make_unique<graph::Router<RouteWeight>>(graph::Router(graph_));
}


size_t TransportRouter::CountStops(const transport_catalogue::TransportCatalogue& catalogue_) {
	size_t stops_counter = 0;
	const auto& stops = catalogue_.GetAlltStops();
	stopname_id_.reserve(stops.size());
	id_stopname_.reserve(stops.size());
	for (const auto& stop : stops) {
		stopname_id_.insert({ stop.first, stops_counter });
		id_stopname_.insert({ stops_counter++, stop.first });
	}
	return stops_counter;
}

double TransportRouter::ComputeRouteTime(const transport_catalogue::TransportCatalogue& catalogue_, 
										domain::Stop* stop_from_index, domain::Stop* stop_to_index) {
	auto distance = catalogue_.GetStopsDistance({ stop_from_index, stop_to_index });
	return ((distance / 1000) / (settings_.bus_velocity_)) * 60; // пререводим метры в километры и часы в минуты
}

bool operator<(const RouteWeight& left, const RouteWeight& right) {
	return left.total_time < right.total_time;
}

bool operator>(const RouteWeight& left, const RouteWeight& right) {
	return left.total_time > right.total_time;
}

RouteWeight operator+(const RouteWeight& left, const RouteWeight& right) {
	RouteWeight result;
	result.total_time = left.total_time + right.total_time;
	return result;
}

std::optional <graph::Router<RouteWeight>::RouteInfo> TransportRouter::BuildRouter(const std::string_view stop_name_from, const std::string_view stop_name_to) const {
	if (!router_) {
		return std::nullopt;
	}
	return router_->BuildRoute(stopname_id_.at(stop_name_from), stopname_id_.at(stop_name_to));
}

graph::DirectedWeightedGraph<RouteWeight>& TransportRouter::GetGraph() {
	return graph_;
}

const graph::DirectedWeightedGraph<RouteWeight>& TransportRouter::GetGraph() const {
	return graph_;
}

std::unique_ptr<graph::Router<RouteWeight>>& TransportRouter::GetRouter() {
	return router_;
}

const std::unique_ptr<graph::Router<RouteWeight>>& TransportRouter::GetRouter() const {
	return router_;
}

const RouterSettings& TransportRouter::GetRouterSettings() const {
	return settings_;
}

const std::string_view TransportRouter::GetStopNameFromID(size_t id) const {
	return id_stopname_.at(id);
}


RouterSettings& TransportRouter::GetRouterSettings()
{
	return settings_;
}

}// namespace transport_router

}// namespace tc_project
