#pragma once
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "json.h"

namespace tc_project {

class RequestHandler
{
public:
	
	RequestHandler(transport_catalogue::TransportCatalogue& tc, render::MapRenderer& map, transport_router::TransportRouter& router);

	void DisplayResult(const json::Node& document, std::ostream& output);

private:

	std::vector<domain::Bus*> GetAllBuses();

	transport_catalogue::TransportCatalogue& catalogue_;
	render::MapRenderer& map_;
	transport_router::TransportRouter& router_;
};

json::Node FindStopInfo(transport_catalogue::TransportCatalogue& tc, std::string_view stop_name, int id);
json::Node FindBusInfo(transport_catalogue::TransportCatalogue& tc, std::string_view bus_name, int id);
json::Node FindMapInfo(std::string_view render_obj, int id);
json::Node FindRoureInfo(transport_router::TransportRouter& router, std::string_view from, std::string_view to, int id);


}//namespace tc_project