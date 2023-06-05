#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"

namespace tc_project {

class RequestHandler
{
public:
	
	RequestHandler(transport_catalogue::TransportCatalogue& tc, render::MapRenderer& map);

	void DisplayResult(const json::Node& document, std::ostream& output);

private:

	std::vector<domain::Bus*> GetAllBuses();

	transport_catalogue::TransportCatalogue& catalogue_;
	render::MapRenderer& map_;
};

json::Node FindStopInfo(transport_catalogue::TransportCatalogue& tc, std::string_view stop_name, int id);
json::Node FindBusInfo(transport_catalogue::TransportCatalogue& tc, std::string_view bus_name, int id);
json::Node FindMapInfo(std::string_view render_obj, int id);

}//namespace tc_project