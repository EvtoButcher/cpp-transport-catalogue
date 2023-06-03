#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <sstream>
#include <variant>

namespace svg {

    struct Point 
    {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };


    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext 
    {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };



    // задаёт значение свойства stroke-linecap — тип формы конца линии
    enum class StrokeLineCap 
    {
        BUTT,
        ROUND,
        SQUARE,
    };
    std::ostream& operator << (std::ostream& out, StrokeLineCap value);
    //адаёт значение свойства stroke-linejoin — тип формы соединения линий
    enum class StrokeLineJoin 
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };
    std::ostream& operator << (std::ostream& out, StrokeLineJoin value);

    /*
    * Вспомогательная структура, для задания цвета в формате RGB
    */
    struct Rgb
    {
        Rgb() = default;

        Rgb(uint8_t r, uint8_t g, uint8_t b)
            : red(r)
            , green(g)
            , blue(b)
        {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    /*
    * Вспомогательная структура, для задания цвета в формате RGBA
    * наследуется от RGB
    */
    struct Rgba : public Rgb
    {
        Rgba() = default;

        Rgba(uint8_t r, uint8_t g, uint8_t b, double o)
            : Rgb(r, g, b)
            , opacity(o)
        {}

        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{};

    std::ostream& operator << (std::ostream& out, Color c);

    struct XmlColorMaker {

        std::ostream& out;

        std::ostream& operator () (std::monostate) const {
            return out << std::string("none"); 
        };

        std::ostream& operator () (std::string& color) const {
            return out << color;
        };

        std::ostream& operator () (Rgb& color) const {
            using namespace std::literals;
            return out << "rgb("s << (int)color.red << ","s
                       << (int)color.green << ","s 
                       << (int)color.blue << ")"s;
        };

        std::ostream& operator () (Rgba& color) const {
            using namespace std::literals;
            return out << "rgba("s << (int)color.red << ","s
                       << (int)color.green << ","s
                       << (int)color.blue << ","s
                       << color.opacity << ")"s;
        };
    };

    /*
    * Базовый класс реализует дополнительные характеристик 
    * для некоторых наследников svg::Object
    */
    template<typename Owner>
    class ObjectProps
    {
    public:
        Owner& SetFillColor(const Color& color) {
            fill_color_ = color;
            return AsOwner();
        }
        
        //задает цвет контура 
        Owner& SetStrokeColor(const Color& color) {
            stroke_color_ = color;
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~ObjectProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
            out << " "sv;
        }
    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional <StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };


    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object 
    {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };


    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public ObjectProps<Circle> 
    {
    public:
        Circle() = default;

        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_ = {0.0, 0.0};
        double radius_ = 1.0;
    };


    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public ObjectProps<Polyline> 
    {
    public:
        Polyline() = default;

        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> points_;
    };


    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public ObjectProps<Text>
    {
    public:
        Text() = default;

        Text& SetPosition(Point pos);

        // Задаёт смещение относительно position_ (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point position_ = {0.0, 0.0};
        Point offset_ = {0.0, 0.0};
        std::string font_family_;
        uint32_t font_size_ = 1;
        std::string font_weight_;
        std::string data_;
    };

    //Абстрактный класс, служит для унифицирует работу с группой объектов svg::Object
    class ObjectContainer
    {
    public:
        //Метод Add добавляет в любой класс реалезующий интерфейс 
        //ObjectContainer любой объект-наследник svg::Object.
        template<typename Obj>
        void Add(Obj value) {
            svg_objects_.emplace_back(std::make_unique<Obj>(std::move(value)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        virtual ~ObjectContainer() = default;
        std::vector< std::unique_ptr<Object>> svg_objects_;
    };



    /*
    * Интерфейс служит для реализации отрисовки
    * любых наследников svg::Object
    */
    class Drawable
    {
    public:
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& odj_container) const = 0;
    };


    class Document final : public ObjectContainer
    {
    public:

        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;
    };

}  // namespace svg
