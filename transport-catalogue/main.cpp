#include <iostream>
#include <fstream>
#include <ostream>
#include <string> 

#include "stat_reader.h"
#include "transport_catalogue.h"
#include "log_duration.h"

using namespace std;
using namespace transport_catalogue;

void StartCatalogue(std::istream& input, std::ostream& output)
{
	file_loader::FileLoader file(input);
	TransportCatalogue catalogue;

	for (const auto& [name, stop] : file.GetBusStop()) {
		catalogue.AddStop(name, Coordinates(std::get<0>(stop), std::get<1>(stop)));
	}

	for (const auto& [name, route] : file.GetBusRoute()) {
		catalogue.AddBus(name, route);
	}

	for (auto& [stop_names, distance_to] : file.GetDistances()) {
		catalogue.AddDistanceFromTo(stop_names.first, stop_names.second, distance_to);
	}

	file_unloader::FileUnloader query(catalogue, input);

	query.DisplayResult(output);
}

int main() {

	std::ifstream in("Test.txt");

	std::ofstream out;
	out.open("hello.txt");
	
	StartCatalogue(in, out);

	out.close();

}