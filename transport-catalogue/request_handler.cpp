#include "request_handler.h"

#include <algorithm>

using namespace std::literals;

namespace tc_project {

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& tc, render::MapRenderer& map)
	: catalogue_(tc)
	, map_(map)
{}

void RequestHandler::DisplayResult(const json::Node& document, std::ostream& output)
{
	const json::Array& request = document.AsArray();

	json::Array output_array;
	output_array.reserve(request.size());

	bool map_is_processed = false;
	for (const auto& dict : request) {

		const json::Dict& request_data = dict.AsMap();

		if (!map_is_processed && request_data.at("type"s).AsString() == "Map"sv) {
			std::ostringstream xml_map;
			map_.Render(xml_map, GetAllBuses());
			output_array.emplace_back(FindMapInfo(xml_map.str(), request_data.at("id"s).AsInt()));
			map_is_processed = true;
		}
		else if(request_data.at("type"s).AsString() == "Stop"sv) {
			output_array.emplace_back(FindStopInfo(catalogue_, request_data.at("name"s).AsString(), request_data.at("id"s).AsInt()));
		}
		else {
			output_array.emplace_back(FindBusInfo(catalogue_, request_data.at("name"s).AsString(), request_data.at("id"s).AsInt()));
		}
	}

	json::Print(json::Document(json::Node(std::move(output_array))), output);
}

std::vector<domain::Bus*> RequestHandler::GetAllBuses()
{
	const auto& sort_buses = catalogue_.GetSortedAllBuses();
	if (sort_buses) {
		return sort_buses.value();
	}
	return {};
}


json::Node FindStopInfo(transport_catalogue::TransportCatalogue& tc, std::string_view stop_name, int id)
{
	auto info = tc.GetStopInfo(stop_name);

	if (info) {
		std::vector<std::string> bus_name;
		bus_name.reserve(info->bus_on_route.size());
		
		for (const auto& bus : info->bus_on_route) {
			bus_name.emplace_back(bus->name);
		}

		return json::Dict{ {"buses"s, json::Array{bus_name.begin(), bus_name.end()}}, {"request_id"s, id}};
	}
	return json::Dict{ {"request_id"s, id}
					,{"error_message"s, "not found"s} };
}

json::Node FindBusInfo(transport_catalogue::TransportCatalogue& tc, std::string_view bus_name, int id)
{
	auto info = tc.GetBusInfo(bus_name);

	if (info.has_value()) {
		return json::Dict{ {"curvature"s, info->route_curvature}, 
						   {"request_id"s, id}, {"route_length"s, info->route_length},
						   {"stop_count"s, static_cast<int>(info->stops)}, 
						   {"unique_stop_count"s, static_cast<int>(info->unique_stop)} };
	}

	return json::Dict{ {"request_id"s, id}
					,{"error_message"s, "not found"s} };
}

json::Node FindMapInfo(std::string_view render_obj, int id)
{
	if (!render_obj.empty()) {
		return json::Dict{ {"map"s, std::string(render_obj)},
							{"request_id"s, id}};
	}

	return json::Dict{ {"request_id"s, id}
					,{"error_message"s, "not found"s} };
}

}//namespace tc_project