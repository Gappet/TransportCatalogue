#include "svg.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string_view>

namespace svg {

using namespace std::literals;

std::string ColorPrinter::operator()(std::monostate) { return "none"s; }

std::string ColorPrinter::operator()(std::string color) { return color; }

std::string ColorPrinter::operator()(Rgb color) {
  std::stringstream out_s;
  out_s << "rgb("s;
  out_s << std::to_string(color.red);
  out_s << ","s;
  out_s << std::to_string(color.green);
  out_s << ","s;
  out_s << std::to_string(color.blue);
  out_s << ")"s;
  return out_s.str();
}

std::string ColorPrinter::operator()(Rgba color) {
  std::stringstream out_s;
  out_s << "rgba("s;
  out_s << std::to_string(color.red);
  out_s << ","s;
  out_s << std::to_string(color.green);
  out_s << ","s;
  out_s << std::to_string(color.blue);
  out_s << ","s;
/// не оставляйте закомментириованный код
  //    out_s << std::fixed << std::setprecision(2);
  out_s << (color.opacity);
  out_s << ")"s;
  return out_s.str();
}

void Object::Render(const RenderContext& context) const {
  context.RenderIndent();

  RenderObject(context);

  context.out << std::endl;
}

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
  RenderAttrs(context.out);
  out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
  points_.push_back(point);
  return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  out << "<polyline points="s;
  if (!points_.empty()) {
    out << "\""sv;
    bool first = true;
    for (const auto& i : points_) {
      if (first) {
        first = false;
        out << i.x << "," << i.y;
      } else {
        out << " " << i.x << "," << i.y;
      }
    }
  } else {
    out << "\"";
  }
  out << "\"";
  RenderAttrs(context.out);
  out << "/>"sv;
}

Text& Text::SetPosition(Point pos) {
  pos_ = pos;
  return *this;
}

Text& Text::SetOffset(Point offset) {
  offset_ = offset;
  return *this;
}

Text& Text::SetFontSize(uint32_t size) {
  size_ = size;
  return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
  font_family_ = font_family;
  return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
  font_weight_ = font_weight;
  return *this;
}

Text& Text::SetData(std::string data) {
  data_ = data;
  return *this;
}

void Text::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  out << "<text";

  RenderAttrs(context.out);
  out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"" << offset_.x
      << "\" dy=\"" << offset_.y;
  out << "\" font-size=\"" << size_;
  if (font_family_ != "") {
    out << "\" font-family=\"" << font_family_;
  }

  if (font_weight_ != "") {
    out << "\" font-weight=\"" << font_weight_;
  }

  out << "\">" << data_ << "</text>";
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
  objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv
      << std::endl;
  RenderContext render(out, 2, 2);

  for (auto& i : objects_) {
    i.get()->Render(render);
  }
  out << "</svg>"sv;
}

}  // namespace svg
