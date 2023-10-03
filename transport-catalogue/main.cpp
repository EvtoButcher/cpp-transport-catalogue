#include <iostream>
#include <fstream>
#include <ostream>
#include <string>

#include "json_reader.h"
#include "request_handler.h"
#include "transport_router.h"
#include "serialization.h"
//#include "log_duration.h"

using namespace std;
using namespace tc_project;
using namespace transport_catalogue;
using namespace render;
using namespace transport_router;

void PrintUsage(std::ostream& stream = std::cerr) {
	stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char** argv) {

	if (argc != 2) {
		PrintUsage();
		return 1;
	}

	const std::string_view program_mode(argv[1]);


	if (program_mode == "make_base"sv) {
		ifstream in("MakeBase.txt", std::ios::binary);

		TransportCatalogue tc;
		MapRenderer map;
		TransportRouter router;

		JesonReader input_json(json::Load(in));
		input_json.FiilCatalogue(tc);
		input_json.FillRenderProperties(map.GetRenderProperties());
		input_json.FillRouteProperties(router, tc);


		//ifstream in2("BaseRequests.txt");
		//JesonReader input_json2(json::Load(in2));
		//std::ifstream db_file(input_json2.GetSerializationSettings().AsMap().at("file"s).AsString(), std::ios::binary);
		//RequestHandler hendler(tc, map, router);
		//ofstream out;
		//out.open("hello2.txt");
		//hendler.DisplayResult(input_json2.GetRequestsToCatalogue(), out);


		ofstream out_db(input_json.GetSerializationSettings().AsMap().at("file"s).AsString(), ios::binary);

		if (out_db.is_open()) {
			Serialize(tc, map, router, out_db);
		}
	}
	else if (program_mode == "process_requests"sv) {


		//ifstream in1("MakeBase.txt", std::ios::binary);

		//TransportCatalogue tc1;
		//MapRenderer map1;
		//TransportRouter router1;

		//JesonReader input_json1(json::Load(in1));
		//input_json1.FiilCatalogue(tc1);
		//input_json1.FillRenderProperties(map1.GetRenderProperties());
		//input_json1.FillRouteProperties(router1, tc1);


		ifstream in("BaseRequests.txt");

		MapRenderer map;
		TransportCatalogue tc;
		TransportRouter router;

		JesonReader input_json(json::Load(in));
		std::ifstream db_file(input_json.GetSerializationSettings().AsMap().at("file"s).AsString(), std::ios::binary);

		if (db_file.is_open()) {
			DeSerialize(tc, map, router, db_file);
		}

		//for (const auto& v : router.GetGraph().edges_) {
		//	for (const auto& v2 : router1.GetGraph().edges_) {
		//		v.weight.bus_name;
		//		v.weight.span_count;
		//		v.weight.total_time;

		//	}
		//}

		RequestHandler hendler(tc, map, router);

		ofstream out; 
		out.open("hello.txt");
		hendler.DisplayResult(input_json.GetRequestsToCatalogue(), out);
	}
	else {
		cerr << "incorrect argument!" << endl;
	}

}