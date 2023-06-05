#pragma once
#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <optional>
#include <vector>

namespace tc_project{

namespace render {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

struct RenderProperties
{
    RenderProperties() = default;

    double width_ = 0.0;
    double height_ = 0.0;
    double padding_ = 0.0;
    double line_width_ = 0.0;
    double stop_radius_ = 0.0;
    int bus_label_font_size_ = 0;
    svg::Point bus_label_offset_ = { 0.0, 0.0 };
    int stop_label_font_size_ = 0;
    svg::Point stop_label_offset_ = { 0.0, 0.0 };
    svg::Color underlayer_color_ = {};
    double underlayer_width_ = 0.0;
    std::vector<svg::Color> color_palette_ = {};
};


class SphereProjector {
public:

    SphereProjector() = default;
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_ = 0.0;
    double min_lon_ = 0.0;
    double max_lat_ = 0.0;
    double zoom_coeff_ = 0.0;
};
    


class MapRenderer
{
public:
    MapRenderer() = default;

    void Render(std::ostream& out, std::vector<domain::Bus*> route);

    RenderProperties& GetRenderProperties();

private:
    void AddRouteLine(std::map<std::string_view, std::vector<domain::Stop*>>& map_geo_coords);
    void AddRouteName(std::map<std::string_view, std::pair<geo::Coordinates, geo::Coordinates>>& start_end_of_route);
    void AddStop(std::map<std::string_view, geo::Coordinates>& stop);
    void AddStopName(std::map<std::string_view, geo::Coordinates>& stop);

    RenderProperties properties_ {};
    SphereProjector proj_{};
    svg::Document map_{};
};

}//namecapse render

}//namespace tc_project