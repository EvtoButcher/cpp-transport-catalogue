#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

namespace tc_project {

namespace detail {

struct PairStringViewHasher {
	size_t operator() (const std::pair<std::string_view, std::string_view> stops) const {
		return str_v_hasher(stops.first) + str_v_hasher(stops.second) * 17;
	}

	std::hash<std::string_view> str_v_hasher;
};

}//namespace detail

class JesonReader 
{
public:
	explicit JesonReader(json::Document document);

	const json::Node& GetBaseRequests() const;
	const json::Node& GetRequestsToCatalogue() const;
	const json::Node& GetRenderProperties() const;
	const json::Node& GetRoutingSettings() const;
	
	void FiilCatalogue(transport_catalogue::TransportCatalogue& catalogue);
	void FillRenderProperties(render::RenderProperties& properties);
	void FillRouteProperties(transport_router::TransportRouter& properties, transport_catalogue::TransportCatalogue& catalogue);

private:

	void ReadBus(const json::Dict& bus);
	void ReadStop(const json::Dict& stop); 
	void ReadRoute(const json::Dict& route);
	svg::Color ReadColor(const json::Node& color);

	json::Document input_document_;
	inline static json::Node empty_node_{ nullptr };

	std::deque<std::string> name_ini_;// инициализация имён для карты
	std::deque<std::vector<std::string>> bus_ini_;// инициализация маршрутов для карты

	std::unordered_map<std::string_view, std::pair<double, double>> bus_stop_;
	std::unordered_map<std::string_view, std::pair<std::vector<std::string_view>, bool>> bus_route_;
	std::unordered_map<std::pair<std::string_view, std::string_view>, unsigned int, detail::PairStringViewHasher> distance_between_stops_;
	std::unordered_map<std::string_view, std::string_view> route_from_stop_to_stop;
};

}//namespace tc_pproject