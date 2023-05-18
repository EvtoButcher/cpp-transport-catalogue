#pragma once
#include <unordered_map>
#include <sstream>
#include <string_view>
#include <tuple>
#include <utility>
#include <deque>

namespace transport_catalogue::file_loader {

class FileLoader
{
public:

	FileLoader(std::istream& input);

	std::unordered_map<std::string_view, std::tuple<double, double>> GetBusStop();
	std::unordered_map<std::string_view, std::vector<std::string_view>> GetBusRoute();
	std::unordered_map<std::string_view, std::vector<std::pair<std::string_view, unsigned int>>> GetDistances();

	void ParseQuery(std::string_view line);

private:

	std::deque<std::string> name_ini;// инициализация имён для карты
	std::deque<std::vector<std::string>> bus_ini;// инициализация маршрутов для карты

	std::unordered_map<std::string_view, std::tuple<double, double>> bus_stop_;
	std::unordered_map<std::string_view, std::vector<std::string_view>> route_;
	std::unordered_map<std::string_view, std::vector<std::pair<std::string_view, unsigned int>>> distance_to_stops_;
};

std::pair<std::string_view, std::tuple<double, double, std::vector<std::pair<std::string_view, double>>>>ParseStop(std::string_view& line);
std::pair<std::string_view, std::vector<std::string_view>>ParseBus(std::string_view& line);

}//namespace transport_catalogue::file_loader