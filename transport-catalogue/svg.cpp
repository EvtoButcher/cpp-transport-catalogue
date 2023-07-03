#include "svg.h"

namespace svg {

    using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point)
{
    points_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
        
    out << "<polyline points=\""sv;
    for (int i = 0; i < static_cast<int>(points_.size()); ++i) {
        if (i) {
            out << " "sv;
        }
        out << points_[i].x << ","sv << points_[i].y;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos)
{
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset)
{
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size)
{
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family)
{
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight)
{
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data)
{
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;

    out << "<text"sv;
    RenderAttrs(out);
    out << "x=\""sv << position_.x << "\" "sv << "y=\""sv << position_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" "sv << "dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\" "sv;

    if (!font_family_.empty()) {
        out << "font-family=\""sv << font_family_ << "\" "sv;
    }
    if (!font_weight_.empty()) {
        out << "font-weight=\""sv << font_weight_ << "\" "sv;
    }
    out << ">"sv;

    std::string text;

    for (const char letter : data_)
    {
        switch (letter)
        {
        case '\"':
            text += "&quot;"s;
            break;
        case '\'':
            text += "&apos;"s;
            break;
        case '<':
            text += "&lt;"s;
            break;
        case '>':
            text += "&gt;"s;
            break;
        case '&':
            text += "&amp"s;
            break;
        default:
            text += letter;
        }
    }
    out << std::move(text) << "</text>"sv;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj)
{ 
    svg_objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const
{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << "\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << "\n"sv;

    for (const auto& obj : svg_objects_) {
        obj->Render({ out, 0, 2 });
    }
    out << "</svg>"sv;
}


std::ostream& operator<<(std::ostream& out, StrokeLineCap value) 
{
    using namespace std::literals;
    switch (value)
    {
    case svg::StrokeLineCap::BUTT: out << "butt"sv;  break;
    case svg::StrokeLineCap::ROUND: out << "round"sv;  break;
    case svg::StrokeLineCap::SQUARE: out << "square"sv; break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin value)
{
    using namespace std::literals;
    switch (value)
    {
    case svg::StrokeLineJoin::ARCS: out << "arcs"sv; break;
    case svg::StrokeLineJoin::BEVEL: out << "bevel"sv; break;
    case svg::StrokeLineJoin::MITER: out << "miter"sv; break;
    case svg::StrokeLineJoin::MITER_CLIP: out << "miter-clip"sv; break;
    case svg::StrokeLineJoin::ROUND: out << "round"sv; break;
    }
    return out;
}

std::ostream& operator << (std::ostream& out, Color color) {
    {
        return std::visit(XmlColorMaker{out}, color);
    }
}

}  // namespace svg