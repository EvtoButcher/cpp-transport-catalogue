#include "map_renderer.h"
#include <algorithm>
#include <map>

namespace tc_project {

namespace render {

bool IsZero(double value)
{
	return std::abs(value) < EPSILON;
}

void MapRenderer::Render(std::ostream& out, std::vector<domain::Bus*> buses)
{ 
	std::map<std::string_view, std::pair<geo::Coordinates, geo::Coordinates>> start_end_of_route;// позиции имён начальной и конечной остановки 
	for (const auto route : buses) {

		auto start_end_stop = std::make_pair(route->stop_on_route[0]->coordinates, 
			route->stop_on_route[route->stop_on_route.size() - 1]->coordinates);

		if (!route->is_roundtrip) {
			start_end_stop.second = route->stop_on_route[route->stop_on_route.size() / 2]->coordinates;
		}

		start_end_of_route[route->name] = start_end_stop;
	}

	std::vector<geo::Coordinates> geo_coords;//кординаты всех остановок для проекции шара на плоскость 
	std::map<std::string_view, std::vector<domain::Stop*>> map_geo_coords;//остановки по автобусам 
	std::map<std::string_view, geo::Coordinates> uniq_stops;//уникальные остановки 
	for (const auto route : buses) {
		if (!route->stop_on_route.size()) {
			continue;
		}
		geo_coords.reserve(geo_coords.size() + route->stop_on_route.size());
		std::for_each(route->stop_on_route.begin(), route->stop_on_route.end(),
			[&](const auto& stop) {
				geo_coords.emplace_back(stop->coordinates);
				uniq_stops[stop->name] = stop->coordinates;
			});
		map_geo_coords[route->name] = route->stop_on_route;
	}

	proj_ = std::move(SphereProjector{ geo_coords.begin(), geo_coords.end(), properties_.width_, properties_.height_, properties_.padding_ });

	AddRouteLine(map_geo_coords);

	AddRouteName(start_end_of_route);

	AddStop(uniq_stops);

	AddStopName(uniq_stops);
	
	map_.Render(out);
}

RenderProperties& MapRenderer::GetRenderProperties()
{
	return properties_;
}

void MapRenderer::AddRouteLine(std::map<std::string_view, std::vector<domain::Stop*>>& map_geo_coords)
{
	size_t color_num = 0;
	for (const auto& [_, stop] : map_geo_coords) {
		svg::Polyline route;
		for (const auto coords : stop) {
			route.AddPoint(proj_(coords->coordinates));
		}
		route.SetFillColor(svg::NoneColor);
		route.SetStrokeColor(properties_.color_palette_[color_num]);
		route.SetStrokeWidth(properties_.line_width_);
		route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		map_.Add(route);

		++color_num;
		if (color_num > (properties_.color_palette_.size() - 1)) {
			color_num = 0;
		}
	}
}

void MapRenderer::AddRouteName(std::map<std::string_view, std::pair<geo::Coordinates, geo::Coordinates>>& start_end_of_route)
{
	size_t color_num = 0;

	auto set_text = [&](svg::Text& text, svg::Text& substrate, const svg::Point& screen_coord, std::string_view name) {
		text.SetPosition(screen_coord);
		text.SetOffset(properties_.bus_label_offset_);
		text.SetFontSize(properties_.bus_label_font_size_);
		text.SetFontFamily(std::string("Verdana"));
		text.SetFontWeight(std::string("bold"));
		text.SetData(std::string(name));
		text.SetFillColor(properties_.color_palette_[color_num]);

		substrate.SetPosition(screen_coord);
		substrate.SetOffset(properties_.bus_label_offset_);
		substrate.SetFontSize(properties_.bus_label_font_size_);
		substrate.SetFontFamily(std::string("Verdana"));
		substrate.SetFontWeight(std::string("bold"));
		substrate.SetData(std::string(name));
		substrate.SetFillColor(properties_.underlayer_color_);
		substrate.SetStrokeColor(properties_.underlayer_color_);
		substrate.SetStrokeWidth(properties_.underlayer_width_);
		substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	};

	for (const auto& [name, coords] : start_end_of_route) {

		svg::Text text;
		svg::Text substrate;

		set_text(text, substrate, proj_(coords.first), name);

		map_.Add(substrate);
		map_.Add(text);

		if (coords.first != coords.second) {
			set_text(text, substrate, proj_(coords.second), name);

			map_.Add(substrate);
			map_.Add(text);
		}

		++color_num;
		if (color_num > (properties_.color_palette_.size() - 1)) {
			color_num = 0;
		}
	}
}

void MapRenderer::AddStop(std::map<std::string_view, geo::Coordinates>& stop)
{
	for (const auto& [_, coord] : stop) {

		svg::Circle stop_on_map;

		stop_on_map.SetCenter(proj_(coord));
		stop_on_map.SetRadius(properties_.stop_radius_);
		stop_on_map.SetFillColor(std::string("white"));

		map_.Add(stop_on_map);
		
	}
}

void MapRenderer::AddStopName(std::map<std::string_view, geo::Coordinates>& stop)
{
	for (const auto& [name, coord] : stop) {

		svg::Text stop_name;
		svg::Text substrate;

		stop_name.SetPosition(proj_(coord));
		stop_name.SetOffset(properties_.stop_label_offset_);
		stop_name.SetFontSize(properties_.stop_label_font_size_);
		stop_name.SetFontFamily(std::string("Verdana"));
		stop_name.SetData(std::string(name));
		stop_name.SetFillColor(std::string("black"));

		substrate.SetPosition(proj_(coord));
		substrate.SetOffset(properties_.stop_label_offset_);
		substrate.SetFontSize(properties_.stop_label_font_size_);
		substrate.SetFontFamily(std::string("Verdana"));
		substrate.SetData(std::string(name));
		substrate.SetFillColor(properties_.underlayer_color_);
		substrate.SetStrokeColor(properties_.underlayer_color_);
		substrate.SetStrokeWidth(properties_.underlayer_width_);
		substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		map_.Add(substrate);
		map_.Add(stop_name);
	}
}

}//namespace render

}//namespace tc_project
