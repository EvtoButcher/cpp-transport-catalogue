#include "serialization.h"
#include <functional>
#include <vector>
#include <chrono>
#include <iostream>

//#define LOG_DURATION(x) LogDuration time(x)
//
//class LogDuration
//{
//public:
//	using Clock = std::chrono::steady_clock;
//	LogDuration(std::string name)
//		:start_time(Clock::now())
//		, duration_name(name) {
//	}
//
//	~LogDuration() {
//		using namespace std::literals;
//
//		const auto end_time = Clock::now();
//		const auto dur = end_time - start_time;
//		std::cerr << duration_name << ": "s << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << " ms" << std::endl;
//	}
//
//private:
//	const Clock::time_point start_time;
//	const std::string duration_name;
//};


namespace tc_project {

proto::Stop MakeStopToSerialize(const domain::Stop* stop, const transport_catalogue::TransportCatalogue& tc)
{
	proto::Stop stop_proto;
	stop_proto.set_name(stop->name);
	stop_proto.set_lat(stop->coordinates.lat);
	stop_proto.set_lng(stop->coordinates.lng);

	for (const auto& [name, distance] : tc.GetStopsNearby(stop)) {
		stop_proto.add_near_stop(std::string(name));
		stop_proto.add_distance(distance);
	}
	return stop_proto;
}

proto::Bus MakeBusToSerialize(const domain::Bus* bus, const transport_catalogue::TransportCatalogue& tc)
{
	proto::Bus bus_proto;
	bus_proto.set_name(bus->name);
	bus_proto.set_is_roundtrip(bus->is_roundtrip);

	for (const auto stop_ptr : bus->stop_on_route) {
		*bus_proto.add_stop_on_route() = MakeStopToSerialize(stop_ptr, tc);
	}

	return bus_proto;
}

proto::RenderProperties MakeRenderPropertiesToSerialize(const render::MapRenderer& map)
{
	proto::RenderProperties render_setting_proto;

	const auto& render_setting = map.GetRenderProperties();

	render_setting_proto.set_width(render_setting.width_);
	render_setting_proto.set_height(render_setting.height_);
	render_setting_proto.set_padding(render_setting.padding_);
	render_setting_proto.set_line_width(render_setting.line_width_);
	render_setting_proto.set_stop_radius(render_setting.stop_radius_);
	render_setting_proto.set_bus_label_font_size(render_setting.bus_label_font_size_);
	proto::Point bus_label_offset_tmp;
	bus_label_offset_tmp.set_x(render_setting.bus_label_offset_.x);
	bus_label_offset_tmp.set_y(render_setting.bus_label_offset_.y);
	*render_setting_proto.mutable_bus_label_offset() = std::move(bus_label_offset_tmp);
	render_setting_proto.set_stop_label_font_size(render_setting.stop_label_font_size_);
	proto::Point stop_label_offset_tmp;
	stop_label_offset_tmp.set_x(render_setting.stop_label_offset_.x);
	stop_label_offset_tmp.set_y(render_setting.stop_label_offset_.y);
	*render_setting_proto.mutable_stop_label_offset() = std::move(stop_label_offset_tmp);
	render_setting_proto.set_underlayer_width(render_setting.underlayer_width_);
	
	std::function<proto::Color (const svg::Color&)> get_proto_color = 
		[&](const svg::Color& color)
	{
		proto::Color tmp_color;
		if (std::holds_alternative<std::string>(color)) {
			tmp_color.set_name(std::get<std::string>(color));
		}
		else if (std::holds_alternative<svg::Rgb>(color)) {
			svg::Rgb rgb_tmp = (std::get<svg::Rgb>(color));
			proto::RGB rgb_proto;
			rgb_proto.set_red(rgb_tmp.red);
			rgb_proto.set_green(rgb_tmp.green);
			rgb_proto.set_blue(rgb_tmp.blue);
			*tmp_color.mutable_rgb() = std::move(rgb_proto);
		}
		else if (std::holds_alternative<svg::Rgba>(color)) {
			svg::Rgba rgba_tmp = (std::get<svg::Rgba>(color));
			proto::RGBA rgba_proto;
			rgba_proto.set_red(rgba_tmp.red);
			rgba_proto.set_green(rgba_tmp.green);
			rgba_proto.set_blue(rgba_tmp.blue);
			rgba_proto.set_opacity(rgba_tmp.opacity);
			*tmp_color.mutable_rgba() = std::move(rgba_proto);
		}
		return tmp_color;
	};

	*render_setting_proto.mutable_underlayer_color() = get_proto_color(render_setting.underlayer_color_);
	
	for (const auto& color : render_setting.color_palette_) {
		*render_setting_proto.add_color_palette() = get_proto_color(color);
	}

	return render_setting_proto;
}

proto::DirectedWeightedGraph MakeGraphToSerialize(const graph::DirectedWeightedGraph<transport_router::RouteWeight>& graph, const transport_catalogue::TransportCatalogue& tc)
{
	proto::DirectedWeightedGraph graph_proto;
	
	for (auto& edge : graph.GetEdges()) {
		proto::Edge proto_edge;
		proto_edge.set_vertex_id_from(edge.from);
		proto_edge.set_vertex_id_to(edge.to);
		proto_edge.mutable_weight()->set_bus_name(std::string(edge.weight.bus_name));
		proto_edge.mutable_weight()->set_total_time(edge.weight.total_time);
		proto_edge.mutable_weight()->set_span_count(edge.weight.span_count);
		*graph_proto.add_edges() = std::move(proto_edge);
	}
	
	for (auto& list : graph.GetIncidenceLists()) {
		auto proto_list = graph_proto.add_incidence_list();
		for (auto id : list) {
			proto_list->add_edges_id(id);
		}
	}

	return graph_proto;
}

proto::Router MakeRouterToSerialize(const std::unique_ptr<graph::Router<transport_router::RouteWeight>>& router)
{
	proto::Router router_proto;

	for (const auto& data : router->GetRoutesInternalData()) {
		proto::RoutesInternalData p_data;
		for (const auto& internal: data) {
		proto::OptionalRouteInternalData p_internal;
			if (internal.has_value()) {
				auto& value = internal.value();
				auto p_value = p_internal.mutable_data();
				p_value->mutable_route_weight()->set_total_time(value.weight.total_time);
				if (value.prev_edge.has_value()) {
					p_value->mutable_prev_edge()->set_edge_id(value.prev_edge.value());
				}
			}
		*p_data.add_routes_internal_data() = std::move(p_internal);
		}
		*router_proto.add_routes_data() = std::move(p_data);
	}
	return router_proto;
}



proto::TransportRouter MakeTransportRouterToSerialize(const transport_router::TransportRouter& router, const transport_catalogue::TransportCatalogue& tc)
{
	proto::TransportRouter router_proto;

	router_proto.mutable_settings()->set_bus_velocity(router.GetRouterSettings().bus_velocity_);
	router_proto.mutable_settings()->set_bus_wait_time(router.GetRouterSettings().bus_wait_time_);
	//*router_proto.mutable_graph() = std::move(MakeGraphToSerialize(router.GetGraph(), tc));
	//*router_proto.mutable_router() = std::move(MakeRouterToSerialize(router.GetRouter()));

	return router_proto;
}


void Serialize(const transport_catalogue::TransportCatalogue& tc, const render::MapRenderer& map, const transport_router::TransportRouter& router, std::ostream& out)
{
	proto::TransportCatalogue tc_db;
	    
	for (const auto [name, stop_ptr] : tc.GetAlltStops()) {
		*tc_db.add_list_of_stops() = std::move(MakeStopToSerialize(stop_ptr, tc));
	}

	for (const auto [name, bus_ptr] : tc.GetAllBuses()) {
		*tc_db.add_list_of_buses() = std::move(MakeBusToSerialize(bus_ptr, tc));
	}

	*tc_db.mutable_render_setting() = std::move(MakeRenderPropertiesToSerialize(map));

	*tc_db.mutable_router() = std::move(MakeTransportRouterToSerialize(router, tc));

	tc_db.SerializeToOstream(&out);
}





void DeSerializeTransportCatalogue(transport_catalogue::TransportCatalogue& tc, const proto::TransportCatalogue& tc_proto)
{
	for (const auto& stop : tc_proto.list_of_stops()) {
		tc.AddStop(stop.name(), geo::Coordinates(stop.lat(), stop.lng()));
	}

	for (const auto& bus : tc_proto.list_of_buses()) {

		std::vector<std::string> stops_sv;
		stops_sv.reserve(bus.stop_on_route_size());
		for (const auto stop : bus.stop_on_route()) {
			stops_sv.push_back(stop.name());
		}

		tc.AddBus(bus.name(), {stops_sv.begin(), stops_sv.end()}, bus.is_roundtrip());
	}

	for (const auto& stop : tc_proto.list_of_stops()) {
		for (int i = 0; i < stop.near_stop_size(); i++) {
			tc.AddDistanceFromTo(stop.name(), stop.near_stop(i), stop.distance(i));
		}
	}
}

void DeSerializeRenderProperties(render::RenderProperties& render_seting, const proto::RenderProperties& render_seting_proto)
{
	render_seting.width_ = render_seting_proto.width();
	render_seting.height_ = render_seting_proto.height();
	render_seting.padding_ = render_seting_proto.padding();
	render_seting.line_width_ = render_seting_proto.line_width();
	render_seting.stop_radius_ = render_seting_proto.stop_radius();
	render_seting.underlayer_width_ = render_seting_proto.underlayer_width();
	render_seting.bus_label_font_size_ = render_seting_proto.bus_label_font_size();
	render_seting.stop_label_font_size_ = render_seting_proto.stop_label_font_size();
	render_seting.bus_label_offset_.x = render_seting_proto.bus_label_offset().x();
	render_seting.bus_label_offset_.y = render_seting_proto.bus_label_offset().y();
	render_seting.stop_label_offset_.x = render_seting_proto.stop_label_offset().x();
	render_seting.stop_label_offset_.y = render_seting_proto.stop_label_offset().y();

	std::function<svg::Color(const proto::Color&)> get_svg_color =
		[&](proto::Color color)
	{
		if (color.has_rgb()) {
			return svg::Color(svg::Rgb(color.rgb().red(), color.rgb().green(), color.rgb().blue()));
		}
		else if (color.has_rgba()) {
			return svg::Color(svg::Rgba(color.rgba().red(), color.rgba().green(), color.rgba().blue(), color.rgba().opacity()));
		}
		else {
			return svg::Color(static_cast<std::string>(color.name()));
		}
	};

	render_seting.underlayer_color_ = get_svg_color(render_seting_proto.underlayer_color());

	render_seting.color_palette_.reserve(render_seting_proto.color_palette_size());
	for (const auto& color : render_seting_proto.color_palette()) {
		render_seting.color_palette_.push_back(std::move(get_svg_color(color)));
	}
}


void DeSerializeTransportRouter(transport_router::TransportRouter& router, const proto::TransportCatalogue& tc_proto, const transport_catalogue::TransportCatalogue& tc)
{
	if (!tc_proto.has_router()) {
		return;
	}

	auto& p_settings = tc_proto.router().settings();

	router.AddRouterSetting({p_settings.bus_wait_time(), p_settings.bus_velocity()});

	router.InicializeGraph(tc);
	
}

void DeSerialize(transport_catalogue::TransportCatalogue& tc, render::MapRenderer& map, transport_router::TransportRouter& router, std::istream& in)
{
	//LOG_DURATION("DeSerialize");

	proto::TransportCatalogue tc_proto;

	if (!tc_proto.ParseFromIstream(&in)) {
		std::cerr << "Parse failed" << '\n' << "Desialization failed" << std::endl;
		return;
	}

	DeSerializeTransportCatalogue(tc, tc_proto);

	DeSerializeRenderProperties(map.GetRenderProperties(), tc_proto.render_setting());

	DeSerializeTransportRouter(router, tc_proto, tc);
}


}//namespace tc_project