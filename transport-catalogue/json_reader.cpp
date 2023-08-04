#include <algorithm>

#include "json_reader.h"

using namespace std::literals;

namespace tc_project {

JesonReader::JesonReader(json::Document document)
	:input_document_(document)
{
	const json::Array& requests = GetBaseRequests().AsArray();

	for (const auto& request_node : requests) {
		const json::Dict& request_map = request_node.AsMap();
		const std::string_view type = request_map.at("type"s).AsString();

		if (type == "Bus"sv) {
			ReadBus(request_map);
		}
		else if(type == "Stop"sv) {
			ReadStop(request_map);
		}
		else {
			ReadRoute(request_map);
		}
	}
}

const json::Node& JesonReader::GetBaseRequests() const
{
	if (input_document_.GetRoot().AsMap().count("base_requests"s)) {
		return input_document_.GetRoot().AsMap().at("base_requests"s);
	}
	return empty_node_;
}

const json::Node& JesonReader::GetRequestsToCatalogue() const
{
	if (input_document_.GetRoot().AsMap().count("stat_requests"s)) {
		return input_document_.GetRoot().AsMap().at("stat_requests"s); 
	}
	return empty_node_;
}

const json::Node& JesonReader::GetRenderProperties() const
{
	if (input_document_.GetRoot().AsMap().count("render_settings"s)) {
		return input_document_.GetRoot().AsMap().at("render_settings"s);
	}
	return empty_node_;
}

const json::Node& JesonReader::GetRoutingSettings() const
{
	if (input_document_.GetRoot().AsMap().count("routing_settings"s)) {
		return input_document_.GetRoot().AsMap().at("routing_settings"s);
	}
	return empty_node_;
}

void JesonReader::FiilCatalogue(transport_catalogue::TransportCatalogue& catalogue)
{

	for (const auto& [name, stop] : bus_stop_) {
		catalogue.AddStop(name, geo::Coordinates(stop.first, stop.second));
	}
	
	for (const auto& [name, route] : bus_route_) {
		catalogue.AddBus(name, route.first, route.second);
	}
	
	for (auto& [stop_names, distance_to] : distance_between_stops_) {
		catalogue.AddDistanceFromTo(stop_names.first, stop_names.second, distance_to);
	}

} 

void JesonReader::FillRenderProperties(render::RenderProperties& properties)
{
	const json::Dict render_properties = GetRenderProperties().AsMap();

	properties.width_ = render_properties.at("width"s).AsDouble();
	properties.height_ = render_properties.at("height"s).AsDouble();
	properties.padding_ = render_properties.at("padding"s).AsDouble();
	properties.line_width_ = render_properties.at("line_width"s).AsDouble();
	properties.stop_radius_ = render_properties.at("stop_radius"s).AsDouble();
	properties.underlayer_width_ = render_properties.at("underlayer_width"s).AsDouble();
	properties.bus_label_font_size_ = render_properties.at("bus_label_font_size"s).AsInt();
	properties.stop_label_font_size_ = render_properties.at("stop_label_font_size"s).AsInt();
	properties.bus_label_offset_.x = render_properties.at("bus_label_offset"s).AsArray()[0].AsDouble();
	properties.bus_label_offset_.y = render_properties.at("bus_label_offset"s).AsArray()[1].AsDouble();
	properties.stop_label_offset_.x = render_properties.at("stop_label_offset"s).AsArray()[0].AsDouble();
	properties.stop_label_offset_.y = render_properties.at("stop_label_offset"s).AsArray()[1].AsDouble();

	properties.underlayer_color_ = ReadColor(render_properties.at("underlayer_color"s));

	const json::Array& colors = render_properties.at("color_palette"s).AsArray();
	properties.color_palette_.reserve(colors.size());
	for (const auto& color : colors) {
		properties.color_palette_.emplace_back(ReadColor(color));
	}
}

void JesonReader::FillRouteProperties(transport_router::TransportRouter& properties, transport_catalogue::TransportCatalogue& catalogue)
{
	const json::Dict route_properties = GetRoutingSettings().AsMap();

	properties.AddRouterSetting({ route_properties.at("bus_wait_time"s).AsDouble(), route_properties.at("bus_velocity"s).AsDouble() });
	properties.InicializeGraph(catalogue);

}

void JesonReader::ReadBus(const json::Dict& bus)
{
	name_ini_.emplace_back(bus.at("name"s).AsString());
	const auto& stops = bus.at("stops"s).AsArray();

	if (stops.empty()) {
		bus_route_[name_ini_.back()] = {};
		return;
	}

	std::vector<std::string> route_tmp;
	route_tmp.reserve(stops.size());

	for (const auto& stop : stops) {
		route_tmp.push_back(stop.AsString());
	}

	//дополнение обратного пути для не кольцевого маршрута
	bool is_roundtrip = bus.at("is_roundtrip"s).AsBool();
	if (!is_roundtrip) {
		std::vector<std::string> route_to_end(route_tmp.rbegin(), route_tmp.rend());
		route_to_end.erase(route_to_end.begin());
		route_tmp.reserve(route_tmp.size() * 2);
		std::move(route_to_end.begin(), route_to_end.end(), std::back_inserter(route_tmp));
	}

	bus_ini_.push_back(std::move(route_tmp));
	bus_route_[name_ini_.back()] = std::make_pair( std::vector<std::string_view>(bus_ini_.back().begin(), bus_ini_.back().end()), is_roundtrip);

}

void JesonReader::ReadStop(const json::Dict& stop)
{
	std::string_view name = stop.at("name"s).AsString();

	if (!bus_stop_.count(name)) {
		name_ini_.emplace_back(std::string(name));
		bus_stop_[name_ini_.back()];
	}

	bus_stop_[name] = std::make_pair(stop.at("latitude"s).AsDouble(), stop.at("longitude"s).AsDouble());

	if (!stop.at("road_distances"s).AsMap().size()) { return; }

	std::for_each(stop.at("road_distances"s).AsMap().begin(), stop.at("road_distances"s).AsMap().end(),
		[&](auto& other_stop) {
			if (!bus_stop_.count(other_stop.first)) {
				name_ini_.emplace_back(other_stop.first);
				bus_stop_[name_ini_.back()];
			}
			distance_between_stops_[std::make_pair(bus_stop_.find(name)->first,
				bus_stop_.find(other_stop.first)->first)] = other_stop.second.AsInt();
		});
}

void JesonReader::ReadRoute(const json::Dict& route)
{
	std::string_view from = route.at("from"s).AsString();
	std::string_view to = route.at("to"s).AsString();

	if (!route_from_stop_to_stop.count(from)) {
		name_ini_.emplace_back(from);
	}
	route_from_stop_to_stop[name_ini_.back()];
	if (!route_from_stop_to_stop.count(to)) {
		name_ini_.emplace_back(to);
	}
	route_from_stop_to_stop[from] = to;
}

svg::Color JesonReader::ReadColor(const json::Node& color)
{
	if (color.IsString()) {
		return color.AsString();
	}
	else {
		const auto& rgb = color.AsArray();
		if (rgb.size() == 3) {
			return svg::Rgb(rgb[0].AsInt(), rgb[1].AsInt(), rgb[2].AsInt());
		}
		else {
			return svg::Rgba(rgb[0].AsInt(), rgb[1].AsInt(), rgb[2].AsInt(), rgb[3].AsDouble());
		}
	}
}

}//namespace tc_project