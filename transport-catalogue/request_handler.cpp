#include "request_handler.h"
#include "json_builder.h"
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

	json::Print(json::Document{json::Builder{}.Value(std::move(output_array)).Build()}, output);
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
		
		return json::Builder{}
					.StartDict()
						.Key("buses"s).Value(json::Array{ bus_name.begin(), bus_name.end() })
						.Key("request_id"s).Value(id)
					.EndDict()
					.Build();
	}
	return json::Builder{}
				.StartDict()
					.Key("request_id"s).Value(id)
					.Key("error_message"s).Value("not found"s)
				.EndDict()
				.Build();
}

json::Node FindBusInfo(transport_catalogue::TransportCatalogue& tc, std::string_view bus_name, int id)
{
	auto info = tc.GetBusInfo(bus_name);

	if (info.has_value()) {
		
		return json::Builder{}
					.StartDict()
						.Key("curvature"s).Value(info->route_curvature)
						.Key("request_id"s).Value(id)
						.Key("route_length"s).Value(info->route_length)
						.Key("stop_count"s).Value(static_cast<int>(info->stops))
						.Key("unique_stop_count"s).Value(static_cast<int>(info->unique_stop))
					.EndDict()
					.Build();
	}

	return json::Builder{}
				.StartDict()
					.Key("request_id"s).Value(id)
					.Key("error_message"s).Value("not found"s)
				.EndDict()
				.Build();
}

json::Node FindMapInfo(std::string_view render_obj, int id)
{
	if (!render_obj.empty()) {
		return json::Builder{}
					.StartDict()
						.Key("map"s).Value(std::string(render_obj))
						.Key("request_id"s).Value(id)
					.EndDict()
					.Build();
	}

	return json::Builder{}
				.StartDict()
					.Key("request_id"s).Value(id)
					.Key("error_message"s).Value("not found"s)
				.EndDict()
				.Build();
}

}//namespace tc_project