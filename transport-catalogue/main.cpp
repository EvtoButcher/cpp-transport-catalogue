#include <iostream>
#include <fstream>
#include <ostream>
#include <string> 

#include "json_reader.h"
#include "request_handler.h"
#include "log_duration.h"

using namespace std;
using namespace tc_project;
using namespace transport_catalogue;
using namespace render;

int main() {

	ifstream in("Test.txt");

	TransportCatalogue tc;
	MapRenderer map;

	JesonReader input_json(json::Load(in));
	input_json.FiilCatalogue(tc);
	input_json.FillRenderProperties(map.GetRenderProperties());


	ofstream out;
	out.open("hello.txt");

	RequestHandler hendler(tc, map);
	hendler.DisplayResult(input_json.GetRequestsToCatalogue(), out);

	out.close();

}