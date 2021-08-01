#include "json_reader.h"

#include <algorithm>
#include <iostream>
#include <tuple>
#include <utility>

#include "geo.h"
#include "json.h"
#include "map_renderer.h"

namespace transpot_guide {
namespace input {
using namespace std::string_literals;

void InputData(TransportCatalogue& transport_catalog, json::Array data) {
  std::vector<json::Node> buses;
  std::vector<json::Node> stops;

  for (const auto& stop_or_bus : data) {
    if (stop_or_bus.AsDict().at("type"s) == "Bus"s) {
      buses.push_back(std::move(stop_or_bus.AsDict()));
    } else {
      stops.push_back(std::move(stop_or_bus.AsDict()));
    }
  }

  size_t cnt_id = 0;
  for (const auto& stop : stops) {
    json::Dict data_of_stop = stop.AsDict();
    transport_catalog.AddStop(data_of_stop["name"s].AsString(),
                              {data_of_stop["latitude"s].AsDouble(),
                               data_of_stop["longitude"s].AsDouble()}, cnt_id);
    ++cnt_id;
  }

  for (auto& stop : stops) {
    json::Dict data_of_stop = stop.AsDict();
    if (data_of_stop.count("road_distances"s)) {
      for (auto& i : data_of_stop["road_distances"s].AsDict()) {
        transport_catalog.AddDistance(data_of_stop["name"s].AsString(), i.first,
                                      i.second.AsInt());
      }
    }
  }

  for (auto& bus : buses) {
    std::vector<std::string_view> stops_;
    for (const json::Node& stop : bus.AsDict().at("stops"s).AsArray()) {
      stops_.push_back(stop.AsString());
    }
    transport_catalog.AddRoute(bus.AsDict().at("name"s).AsString(), stops_,
                               bus.AsDict().at("is_roundtrip"s).AsBool());
  }
}

svg::Color ParsingColor(const json::Node& color) {
  svg::Color out;
  if (color.IsString()) {
    return color.AsString();
  }

  if (color.AsArray().size() == 3) {
    return svg::Rgb(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(),
                    color.AsArray()[2].AsInt());
  }

  return svg::Rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(),
                   color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble());
}

RenderSettings ReadRenderSettings(json::Dict data) {
  RenderSettings settings;
  settings.width = data["width"s].AsDouble();
  settings.height = data["height"s].AsDouble();
  settings.padding = data["padding"s].AsDouble();
  settings.line_width = data["line_width"s].AsDouble();
  settings.stop_radius = data["stop_radius"s].AsDouble();
  settings.bus_label_front_size = data["bus_label_font_size"s].AsDouble();

  auto bus_label_offset = data["bus_label_offset"s];
  settings.bus_label_offset =
      svg::Point(bus_label_offset.AsArray()[0].AsDouble(),
                 bus_label_offset.AsArray()[1].AsDouble());
  settings.stop_label_font_size = data["stop_label_font_size"s].AsInt();

  auto stop_label_offset = data["stop_label_offset"s];
  settings.stop_label_offset =
      svg::Point(stop_label_offset.AsArray()[0].AsDouble(),
                 stop_label_offset.AsArray()[1].AsDouble());
  settings.underlayer_color = ParsingColor(data["underlayer_color"s]);
  settings.underlayer_width = data["underlayer_width"s].AsDouble();
  for (const auto& color : data.at("color_palette"s).AsArray()) {
    settings.color_palette.push_back(ParsingColor(color));
  }
  return settings;
}
}  // namespace input
}  // namespace transpot_guide
