#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>
#include <svg.pb.h>
#include <map_renderer.pb.h>
#include <tuple>

namespace tc_project {

static std::unordered_map<int, std::string_view> stop_id_to_name_;

proto::Stop MakeStopToSerialize(const domain::Stop* stop, const transport_catalogue::TransportCatalogue& tc);

proto::Bus MakeBusToSerialize(const domain::Bus* bus, const transport_catalogue::TransportCatalogue& tc);

proto::RenderProperties MakeRenderPropertiesToSerialize(const render::MapRenderer& map);

proto::DirectedWeightedGraph MakeGraphToSerialize(const graph::DirectedWeightedGraph<transport_router::RouteWeight>& graph, const transport_catalogue::TransportCatalogue& tc);

proto::Router MakeRouterToSerialize(const std::unique_ptr<graph::Router<transport_router::RouteWeight>>& router);

proto::TransportRouter MakeTransportRouterToSerialize(const transport_router::TransportRouter& router, const transport_catalogue::TransportCatalogue& tc);


void Serialize(const transport_catalogue::TransportCatalogue& tc, const render::MapRenderer& map, const transport_router::TransportRouter& router, std::ostream& out);

void DeSerializeTransportCatalogue(transport_catalogue::TransportCatalogue& tc, const proto::TransportCatalogue& tc_proto);

void DeSerializeRenderProperties(render::RenderProperties& render_seting, const proto::RenderProperties& render_seting_proto);

void DeSerializeTransportRouter(transport_router::TransportRouter& router, const proto::TransportCatalogue& tc_proto, const transport_catalogue::TransportCatalogue& tc);

void DeSerialize(transport_catalogue::TransportCatalogue& tc, render::MapRenderer& map, transport_router::TransportRouter& router, std::istream& in);

	
}//namespace tc_project