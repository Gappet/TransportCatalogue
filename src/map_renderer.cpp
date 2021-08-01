#include "map_renderer.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "json_builder.h"

using namespace std::literals;

bool IsZero(double value) { return std::abs(value) < EPSILON; }

SphereProjector CreatorSphereProjector(std::deque<transpot_guide::Stop*>& stops,
                                       RenderSettings& settings) {
  std::vector<transpot_guide::detail::Coordinates> points;
  for (auto stop : stops) {
    points.push_back(stop->coordinates);
  }
  return SphereProjector(points.begin(), points.end(), settings.width,
                         settings.height, settings.padding);
}

std::vector<svg::Polyline> DrawLineofRoad(
    const std::deque<transpot_guide::Bus*>& buses, RenderSettings& settings,
    SphereProjector& projector) {
  std::vector<svg::Polyline> lines;
  size_t cnt_color_palette = 0;
  for (const auto bus : buses) {
    if (!bus->route_stops.empty()) {
      svg::Polyline line;
      for (auto& stop : bus->route_stops) {
        line.AddPoint(projector(stop->coordinates));
        /// несколько не понятно почему этот и последующие методы в цикле
        /// (stop), вроде устанавливают свойство у line и свойство вроде как не
        /// зависит от добавленной остановки но может чего просмотрел
        line.SetStrokeColor(settings.color_palette[cnt_color_palette]);
        line.SetStrokeWidth(settings.line_width);
        line.SetFillColor(svg::NoneColor);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
      }

      if (!bus->is_roundtrip) {
        for (auto itr = bus->route_stops.rbegin() + 1;
             itr < bus->route_stops.rend(); ++itr) {
          line.AddPoint(projector((*itr)->coordinates));
        }
      }

      if (cnt_color_palette < settings.color_palette.size() - 1) {
        ++cnt_color_palette;
      } else {
        cnt_color_palette = 0;
      }
      lines.push_back(line);
    }
  }
  return lines;
}

std::vector<svg::Text> DrawNameOfRoad(
    const std::deque<transpot_guide::Bus*>& buses, RenderSettings& settings,
    SphereProjector& projector) {
  std::vector<svg::Text> NameOfRoad;
  size_t cnt_color_palette = 0;
  for (const auto bus : buses) {
    if (!bus->route_stops.empty()) {
      if (bus->is_roundtrip ||
          bus->route_stops.front()->name == bus->route_stops.back()->name) {
        svg::Text text_first;
        svg::Text text_second;
        text_first.SetPosition(projector(bus->route_stops[0]->coordinates))
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_front_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold")
            .SetData(bus->name)
            .SetFillColor(settings.color_palette[cnt_color_palette]);

        text_second.SetPosition(projector(bus->route_stops[0]->coordinates))
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_front_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold")
            .SetData(bus->name)
            .SetFillColor(settings.underlayer_color)
            .SetStrokeColor(settings.underlayer_color)
            .SetStrokeWidth(settings.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        NameOfRoad.push_back(text_second);
        NameOfRoad.push_back(text_first);

      } else {
        svg::Text text_first_first_stop;
        svg::Text text_second_first_stop;
        svg::Text text_first_secon_stop;
        svg::Text text_second_second_stop;

        text_first_first_stop
            .SetPosition(projector(bus->route_stops[0]->coordinates))
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_front_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s)
            .SetData(bus->name)
            .SetFillColor(settings.color_palette[cnt_color_palette]);

        text_second_first_stop
            .SetPosition(projector(bus->route_stops[0]->coordinates))
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_front_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s)
            .SetData(bus->name)
            .SetFillColor(settings.underlayer_color)
            .SetStrokeColor(settings.underlayer_color)
            .SetStrokeWidth(settings.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text_first_secon_stop
            .SetPosition(projector(
                bus->route_stops[(bus->route_stops.size() - 1)]->coordinates))
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_front_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s)
            .SetData(bus->name)
            .SetFillColor(settings.color_palette[cnt_color_palette]);

        text_second_second_stop
            .SetPosition(projector(
                bus->route_stops[(bus->route_stops.size() - 1)]->coordinates))
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_front_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s)
            .SetData(bus->name)
            .SetFillColor(settings.underlayer_color)
            .SetStrokeColor(settings.underlayer_color)
            .SetStrokeWidth(settings.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        NameOfRoad.push_back(text_second_first_stop);
        NameOfRoad.push_back(text_first_first_stop);

        NameOfRoad.push_back(text_second_second_stop);
        NameOfRoad.push_back(text_first_secon_stop);
      }

      if (cnt_color_palette < settings.color_palette.size() - 1) {
        ++cnt_color_palette;
      } else {
        cnt_color_palette = 0;
      }
    }
  }
  return NameOfRoad;
}

std::vector<svg::Circle> DrawStop(
    const std::deque<transpot_guide::Stop*>& stops, RenderSettings& settings,
    SphereProjector& projector) {
  std::vector<svg::Circle> stops_point;
  for (const auto stop : stops) {
    svg::Circle stop_point;
    stop_point.SetCenter(projector(stop->coordinates))
        .SetRadius(settings.stop_radius)
        .SetFillColor("white"s);
    stops_point.push_back(stop_point);
  }

  return stops_point;
}

std::vector<svg::Text> DrawStopName(
    const std::deque<transpot_guide::Stop*>& stops, RenderSettings& settings,
    SphereProjector& projector) {
  std::vector<svg::Text> stops_name;
  for (const auto stop : stops) {
    svg::Text first_text;
    svg::Text second_text;
    first_text.SetPosition(projector(stop->coordinates))
        .SetOffset(settings.stop_label_offset)
        .SetFontSize(settings.stop_label_font_size)
        .SetFontFamily("Verdana"s)
        .SetData(stop->name)
        .SetFillColor("black"s);
    second_text.SetPosition(projector(stop->coordinates))
        .SetOffset(settings.stop_label_offset)
        .SetFontSize(settings.stop_label_font_size)
        .SetFontFamily("Verdana"s)
        .SetData(stop->name)
        .SetFillColor(settings.underlayer_color)
        .SetStrokeColor(settings.underlayer_color)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
        .SetStrokeWidth(settings.underlayer_width);
    stops_name.push_back(second_text);
    stops_name.push_back(first_text);
  }
  return stops_name;
}

json::Node GetMapOfRoad(::transpot_guide::TransportCatalogue& transport_catalog,
                        RenderSettings& settings, int id) {
  auto raw_stops = transport_catalog.GetStops();
  std::deque<transpot_guide::Stop*> stops;
  for (auto stop : raw_stops) {
    if (!transport_catalog.GetBusofStop(stop->name).empty()) {
      stops.push_back(stop);
    }
  }

  SphereProjector projector = CreatorSphereProjector(stops, settings);

  auto buses = transport_catalog.GetBus();
  std::sort(buses.begin(), buses.end(), [](auto& left, auto& right) {
    return std::lexicographical_compare(left->name.begin(), left->name.end(),
                                        right->name.begin(), right->name.end());
  });

  std::vector<svg::Polyline> lines = DrawLineofRoad(buses, settings, projector);
  std::vector<svg::Text> NamesOfRoad =
      DrawNameOfRoad(buses, settings, projector);

  std::sort(stops.begin(), stops.end(), [](auto left, auto right) {
    return std::lexicographical_compare(left->name.begin(), left->name.end(),
                                        right->name.begin(), right->name.end());
  });

  std::vector<svg::Circle> stop_points = DrawStop(stops, settings, projector);

  std::vector<svg::Text> stop_names = DrawStopName(stops, settings, projector);

  svg::Document doc;

  for (auto& line : lines) {
    doc.Add(std::move(line));
  }

  for (auto& name : NamesOfRoad) {
    doc.Add(std::move(name));
  }

  for (auto& point : stop_points) {
    doc.Add(std::move(point));
  }

  for (auto& name : stop_names) {
    doc.Add(std::move(name));
  }

  std::stringstream svg_data;
  doc.Render(svg_data);
  auto out = json::Builder{}
                 .StartDict()
                 .Key("request_id"s)
                 .Value(id)
                 .Key("map"s)
                 .Value(svg_data.str())
                 .EndDict()
                 .Build();
  return out;
}
