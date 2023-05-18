#pragma once
#include <sstream>
#include <deque>
#include <unordered_map>
#include <string_view>
#include <string>

#include "transport_catalogue.h"

namespace transport_catalogue {
	
namespace file_unloader{

struct Query
{
	Query() = default;

	Query(char k, std::string_view n)
		: key(k)
		, name(n)
	{};

	char key;
	std::string_view name;
};

class FileUnloader
{
public:
	FileUnloader(TransportCatalogue& catalogue, std::istream& input);

	void GetResult(std::ostream& output);
	Query ParseQuery(std::string_view line);

private:
	size_t number_of_queryies = 0;

	std::vector<Query> fresh_queryies_;

	std::deque<std::string> bus_queryies_;
	std::unordered_map<std::string_view, detail::BusInfo> processed_bus_queries_;

	std::deque<std::string> stop_queryies_;
	std::unordered_map<std::string_view, detail::StopInfo> processed_stop_queries_;
};

}//namespase file_unloader

}//namespace transport_catalogue