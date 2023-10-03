#pragma once
#include "transport_catalogue.h"
#include "router.h"

#include <memory>

namespace tc_project {

namespace transport_router {

constexpr static double KM_TO_MIN_FACTOR = 1000.0 / 60.0;

struct RouteWeight {
	std::string_view bus_name{};
	double total_time = 0;
	int span_count = 0;
};

struct RouterSettings {
	RouterSettings() = default;

	RouterSettings(double bus_wait_time, double bus_velocity)
		: bus_wait_time_(bus_wait_time)
		, bus_velocity_(bus_velocity) {};

	double bus_wait_time_ = 0.0;
	double bus_velocity_ = 0.0;
};

class TransportRouter {
public:

	TransportRouter() = default;

	std::optional <graph::Router<RouteWeight>::RouteInfo> BuildRouter(const std::string_view stop_name_from, const std::string_view stop_name_to) const;

	graph::DirectedWeightedGraph<RouteWeight>& GetGraph();
	const graph::DirectedWeightedGraph<RouteWeight>& GetGraph() const;

	std::unique_ptr<graph::Router<RouteWeight>>& GetRouter();
	const std::unique_ptr<graph::Router<RouteWeight>>& GetRouter() const;

	const RouterSettings& GetRouterSettings() const;
	const std::string_view GetStopNameFromID(size_t id) const;

	void AddRouterSetting(RouterSettings settings);

	void InicializeGraph(const transport_catalogue::TransportCatalogue& catalogue_);

	RouterSettings& GetRouterSettings();

private:

	RouterSettings settings_{};

	std::unordered_map<std::string_view, size_t> stopname_id_;

	std::unordered_map<uint32_t, std::string_view> id_stopname_;

	std::unique_ptr<graph::Router<RouteWeight>> router_ = nullptr;

	graph::DirectedWeightedGraph<RouteWeight> graph_;

	bool CheckArgument(double arg);

	double ComputeRouteTime(const transport_catalogue::TransportCatalogue& catalogue_, domain::Stop* stop_from_index, domain::Stop* stop_to_index);

	void BuildGraph(graph::DirectedWeightedGraph<RouteWeight>& graph, const transport_catalogue::TransportCatalogue& catalogue_,
		const std::vector<domain::Stop*>& stops, const std::string_view bus_name);

	size_t CountStops(const transport_catalogue::TransportCatalogue& catalogue_);
};

bool operator<(const RouteWeight& left, const RouteWeight& right);
bool operator>(const RouteWeight& left, const RouteWeight& right);
RouteWeight operator+(const RouteWeight& left, const RouteWeight& right);


}//transport_router

}//namespace tc_project 
