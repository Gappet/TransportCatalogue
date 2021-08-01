#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

struct Point {
  Point() = default;
  Point(double x, double y) : x(x), y(y) {}
  double x = 0;
  double y = 0;
};

struct Rgb {
  Rgb() = default;
  Rgb(uint8_t red, uint8_t green, uint8_t blue)
      : red(red), green(green), blue(blue) {}
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
};

struct Rgba {
  Rgba() = default;
  Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
      : red(red), green(green), blue(blue), opacity(opacity) {}
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  double opacity = 1.0;
};

enum class StrokeLineCap { BUTT, ROUND, SQUARE };

enum class StrokeLineJoin { ARCS, BEVEL, MITER, MITER_CLIP, ROUND };

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor = {};

struct ColorPrinter {
  std::string operator()(std::monostate);

  std::string operator()(std::string color);

  std::string operator()(Rgb color);

  std::string operator()(Rgba color);
};

inline std::ostream& operator<<(std::ostream& out, const StrokeLineCap& other) {
  switch (other) {
    case StrokeLineCap::BUTT:
      out << "butt";
      break;

    case StrokeLineCap::ROUND:
      out << "round";
      break;

    case StrokeLineCap::SQUARE:
      out << "square";
      break;
  }

  return out;
}

inline std::ostream& operator<<(std::ostream& out,
                                const StrokeLineJoin& other) {
  switch (other) {
    case StrokeLineJoin::ARCS:
      out << "arcs";
      break;

    case StrokeLineJoin::BEVEL:
      out << "bevel";
      break;

    case StrokeLineJoin::MITER:
      out << "miter";
      break;

    case StrokeLineJoin::MITER_CLIP:
      out << "miter-clip";
      break;

    case StrokeLineJoin::ROUND:
      out << "round";
      break;
  }

  return out;
}

template <typename Owner>
class PathProps {
 public:
  Owner& SetFillColor(Color color) {
    fill_color_ = std::move(color);
    return AsOwner();
  }
  Owner& SetStrokeColor(Color color) {
    stroke_color_ = std::move(color);
    return AsOwner();
  }

  Owner& SetStrokeWidth(double width) {
    stroke_width_ = std::move(width);
    return AsOwner();
  }

  Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
    stroke_linecap_ = std::move(line_cap);
    return AsOwner();
  }

  Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
    stroke_linejoin_ = std::move(line_join);
    return AsOwner();
  }

 protected:
  ~PathProps() = default;

  void RenderAttrs(std::ostream& out) const {
    using namespace std::literals;

    if (fill_color_) {
      out << " fill=\""sv;
      out << std::visit(ColorPrinter{}, *fill_color_);
      out << "\""sv;
    }
    if (stroke_color_) {
      out << " stroke=\""sv;
      out << std::visit(ColorPrinter{}, *stroke_color_);
      out << "\""sv;
    }

    if (stroke_width_) {
      out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
    }

    if (stroke_linecap_) {
      out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
      ;
    }

    if (stroke_linejoin_) {
      out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
      ;
    }
  }

 private:
  Owner& AsOwner() {
    // static_cast безопасно преобразует *this к Owner&,
    // если класс Owner — наследник PathProps
    return static_cast<Owner&>(*this);
  }

  std::optional<Color> fill_color_;
  std::optional<Color> stroke_color_;
  std::optional<double> stroke_width_;
  std::optional<StrokeLineCap> stroke_linecap_;
  std::optional<StrokeLineJoin> stroke_linejoin_;
};

struct RenderContext {
  RenderContext(std::ostream& out) : out(out) {}

  RenderContext(std::ostream& out, int indent_step, int indent = 0)
      : out(out), indent_step(indent_step), indent(indent) {}

  RenderContext Indented() const {
    return {out, indent_step, indent + indent_step};
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

class Object {
 public:
  void Render(const RenderContext& context) const;

  virtual ~Object() = default;

 private:
  virtual void RenderObject(const RenderContext& context) const = 0;
};

class Circle final : public Object, public PathProps<Circle> {
 public:
  Circle& SetCenter(Point center);
  Circle& SetRadius(double radius);

 private:
  void RenderObject(const RenderContext& context) const override;

  Point center_;
  double radius_ = 1.0;
};

class Polyline : public Object, public PathProps<Polyline> {
 public:
  // Добавляет очередную вершину к ломаной линии
  Polyline& AddPoint(Point point);

 private:
  void RenderObject(const RenderContext& context) const override;

  std::vector<Point> points_;
};

class Text : public Object, public PathProps<Text> {
 public:
  // Задаёт координаты опорной точки (атрибуты x и y)
  Text& SetPosition(Point pos);

  // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
  Text& SetOffset(Point offset);

  // Задаёт размеры шрифта (атрибут font-size)
  Text& SetFontSize(uint32_t size);

  // Задаёт название шрифта (атрибут font-family)
  Text& SetFontFamily(std::string font_family);

  // Задаёт толщину шрифта (атрибут font-weight)
  Text& SetFontWeight(std::string font_weight);

  // Задаёт текстовое содержимое объекта (отображается внутри тега text)
  Text& SetData(std::string data);

  // Прочие данные и методы, необходимые для реализации элемента <text>

 private:
  void RenderObject(const RenderContext& context) const override;

  Point pos_;
  Point offset_;
  uint32_t size_ = 1;
  std::string font_family_ = "";
  std::string font_weight_ = "";
  std::string data_;
};

class ObjectContainer {
 public:
  template <typename Obj>
  void Add(Obj obj);

  virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

  virtual ~ObjectContainer() = default;

 protected:
  std::vector<std::shared_ptr<Object>> objects_;
};

class Drawable {
 public:
  virtual void Draw(ObjectContainer& objects) const = 0;

  virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
 public:
  void AddPtr(std::unique_ptr<Object>&& obj) override;

  void Render(std::ostream& out) const;
};

template <typename Obj>
void ObjectContainer::Add(Obj obj) {
  AddPtr(std::make_unique<Obj>(std::move(obj)));
}

}  // namespace svg
