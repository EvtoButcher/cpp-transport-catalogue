cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto transport_router.proto graph.proto)

set(TC_FILES 
domain.cpp domain.h 
geo.h 
graph.h 
json.cpp json.h 
json_builder.cpp json_builder.h
json_reader.cpp json_reader.h 
map_renderer.cpp map_renderer.h 
ranges.h 
request_handler.cpp request_handler.h 
router.h 
svg.cpp svg.h 
transport_catalogue.cpp transport_catalogue.h 
transport_router.cpp transport_router.h 
main.cpp 
transport_catalogue.proto 
map_renderer.proto 
graph.proto 
transport_router.proto 
svg.proto
serialization.h serialization.cpp) 

add_executable(transportcatalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TC_FILES})
target_include_directories(transportcatalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transportcatalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transportcatalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
