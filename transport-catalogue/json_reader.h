#pragma once
#include "json.h"
#include "transport_catalogue.h"
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
	
	void FiilCatalogue(transport_catalogue::TransportCatalogue& catalogue);
	void FillRenderProperties(render::RenderProperties& properties);

private:

	void ReadBus(const json::Dict& bus);
	void ReadStop(const json::Dict& stop);
	svg::Color ReadColor(const json::Node& color);

	json::Document input_document_;
	inline static json::Node empty_node_{ nullptr };

	std::deque<std::string> name_ini_;// ������������� ��� ��� �����
	std::deque<std::vector<std::string>> bus_ini_;// ������������� ��������� ��� �����

	std::unordered_map<std::string_view, std::pair<double, double>> bus_stop_;
	std::unordered_map<std::string_view, std::pair<std::vector<std::string_view>, bool>> route_;
	std::unordered_map<std::pair<std::string_view, std::string_view>, unsigned int, detail::PairStringViewHasher> distance_between_stops_;
};

}//namespace tc_pproject