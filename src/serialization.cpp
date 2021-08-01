#include "serialization.h"

#include <transport_catalogue.pb.h>

#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "graph.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "router.h"
#include "svg.h"
#include "transport_catalogue.h"

void SerializeBusAndStop(transpot_guide::TransportCatalogue& transport_catalog,
                         transport_base::TransportCatalogue& base) {
  for (const auto stop : transport_catalog.GetStops()) {
    transport_base::Stop stop_data;

    stop_data.set_id(static_cast<uint32_t>(stop->id));
    stop_data.set_lat(stop->coordinates.lat);
    stop_data.set_lng(stop->coordinates.lng);
    *base.add_stop() = stop_data;
    (*base.mutable_stop_name())[stop->id] = stop->name;
  }

  for (const auto bus : transport_catalog.GetBus()) {
    transport_base::Bus bus_data;
    bus_data.set_name(bus->name);
    bus_data.set_is_roundtrip(bus->is_roundtrip);
    for (auto stop : bus->route_stops) {
      bus_data.add_stops(static_cast<uint32_t>(stop->id));
    }
    *base.add_bus() = bus_data;
  }

  base.set_separator(separator);
  for (auto& [stops, length] : transport_catalog.GetLenghBtwStop()) {
    (*base.mutable_distance())[stops.first->name + separator +
                               stops.second->name] = length;
  }
}

void SerializeRenderSettings(
    transpot_guide::TransportCatalogue& transport_catalog,
    transport_base::TransportCatalogue& base, RenderSettings& settings) {
  transport_base::RenderSettings render_settings;
  render_settings.set_width(settings.width);
  render_settings.set_height(settings.height);
  render_settings.set_padding(settings.padding);
  render_settings.set_stop_radius(settings.stop_radius);
  render_settings.set_line_width(settings.line_width);
  render_settings.set_bus_label_front_size(settings.bus_label_front_size);
  transport_base::Point point_0;
  point_0.set_x(settings.bus_label_offset.x);
  point_0.set_y(settings.bus_label_offset.y);
  *render_settings.mutable_bus_label_offset() = point_0;
  render_settings.set_stop_label_font_size(settings.stop_label_font_size);

  transport_base::Point point_1;
  point_1.set_x(settings.stop_label_offset.x);
  point_1.set_y(settings.stop_label_offset.y);
  *render_settings.mutable_stop_label_offset() = point_1;

  render_settings.set_underlayer_color(
      std::visit(svg::ColorPrinter{}, settings.underlayer_color));
  render_settings.set_underlayer_width(settings.underlayer_width);

  for (const auto clr : settings.color_palette) {
    *render_settings.add_color_palette() = std::visit(svg::ColorPrinter{}, clr);
  }

  *base.mutable_render_settings() = render_settings;
}

void SerializeGraph(transpot_guide::TransportCatalogue& transport_catalog,
                    transport_base::TransportCatalogue& base) {
  auto graph_transpot = transport_catalog.BuildGraph();

  transport_base::Graph graph;
  for (const auto& edge : graph_transpot.GetEdges()) {
    transport_base::Edge edge_out;
    edge_out.set_from(edge.from);
    edge_out.set_to(edge.to);
    edge_out.set_weight(edge.weight);
    *graph.add_edges() = edge_out;
  }

  for (const auto& list : graph_transpot.GetIncidenceLists()) {
    transport_base::IncidenceList incidence_list;
    for (auto edge_id : list) {
      incidence_list.add_edge_id(edge_id);
    }
    *graph.add_ind_lst() = incidence_list;
  }
  *base.mutable_graph() = graph;
}

void SerializeBtwStopInfo(transpot_guide::TransportCatalogue& transport_catalog,
                          transport_base::TransportCatalogue& base) {
  auto btw_stop_info = transport_catalog.GetBtwStopInfo();
  for (auto& [edge_id, weight] : btw_stop_info) {
    transport_base::Weight weight_out;
    weight_out.set_wait_time(weight.wait_time);
    weight_out.set_travel_time(weight.travel_time);
    weight_out.set_span_count(weight.span_count);
    weight_out.set_stop_name(std::string(weight.stop_name));
    weight_out.set_bus(std::string(weight.bus));
    weight_out.set_wait(weight.wait);
    (*base.mutable_btw_stop_info())[edge_id] = weight_out;
  }
}

void MakeBase(transpot_guide::TransportCatalogue& transport_catalog,
              RenderSettings& settings, std::ostream& output) {
  transport_base::TransportCatalogue base;
  SerializeBusAndStop(transport_catalog, base);
  SerializeRenderSettings(transport_catalog, base, settings);
  SerializeGraph(transport_catalog, base);
  SerializeBtwStopInfo(transport_catalog, base);
  base.SerializeToOstream(&output);
}

void DerializeBusAndStop(transpot_guide::TransportCatalogue& transport_catalog,
                         transport_base::TransportCatalogue& base) {
  for (const auto& stop : base.stop()) {
    transport_catalog.AddStop((*base.mutable_stop_name())[stop.id()],
                              {stop.lat(), stop.lng()}, stop.id());
  }

  for (const auto& [pair, lenght] : base.distance()) {
    transport_catalog.AddDistance(
        pair.substr(0, pair.find(separator)),
        pair.substr(pair.find(separator) + separator.size(), pair.size()),
        lenght);
  }

  for (const auto& bus : base.bus()) {
    std::vector<std::string_view> stops;
    for (const auto& id : bus.stops()) {
      stops.push_back((*base.mutable_stop_name())[id]);
    }
    transport_catalog.AddRoute(bus.name(), stops, bus.is_roundtrip());
  }
}

RenderSettings DserializeRenderSettings(
    transpot_guide::TransportCatalogue& transport_catalog,
    transport_base::TransportCatalogue& base) {
  RenderSettings settings;
  settings.height = (base.render_settings()).height();
  settings.width = (base.render_settings()).width();
  settings.padding = (base.render_settings()).padding();
  settings.stop_radius = (base.render_settings()).stop_radius();
  settings.line_width = (base.render_settings()).line_width();
  settings.bus_label_front_size =
      (base.render_settings()).bus_label_front_size();
  settings.bus_label_offset.x = (base.render_settings()).bus_label_offset().x();
  settings.bus_label_offset.y = (base.render_settings()).bus_label_offset().y();
  settings.stop_label_font_size =
      (base.render_settings()).stop_label_font_size();
  settings.stop_label_offset.x =
      (base.render_settings()).stop_label_offset().x();
  settings.stop_label_offset.y =
      (base.render_settings()).stop_label_offset().y();
  settings.underlayer_width = (base.render_settings()).underlayer_width();
  settings.underlayer_color = (base.render_settings()).underlayer_color();
  settings.underlayer_width = (base.render_settings()).underlayer_width();
  for (auto clr : (base.render_settings()).color_palette()) {
    settings.color_palette.push_back(clr);
  }
  return settings;
}

graph::DirectedWeightedGraph<double> DrserializeGraph(
    transpot_guide::TransportCatalogue& transport_catalog,
    transport_base::TransportCatalogue& base) {
  graph::DirectedWeightedGraph<double> graph;
  for (auto& edge : base.graph().edges()) {
    graph.GetEdges().push_back({edge.from(), edge.to(), edge.weight()});
  }

  for (auto& list : base.graph().ind_lst()) {
    std::vector<size_t> v;
    for (auto edge_id : list.edge_id()) {
      v.push_back(edge_id);
    }
    graph.GetIncidenceLists().push_back(v);
  }
  return graph;
}

void DeserializeBtwStopInfo(
    transpot_guide::TransportCatalogue& transport_catalog,
    transport_base::TransportCatalogue& base) {
  // auto& btw_stop_info = transport_catalog.GetBtwStopInfo();
  std::map<graph::EdgeId, transpot_guide::TransportCatalogue::Weight> info;
  for (auto& [edge_id, weight] : base.btw_stop_info()) {
    info[edge_id] = {weight.wait_time(),  weight.travel_time(),
                     weight.span_count(), weight.stop_name(),
                     weight.bus(),        weight.wait()};
  }

  transport_catalog.SetBtwStopInfo(info);
}

json::Node ProcessRequests(const json::Document& query) {
  auto map_ = query.GetRoot().AsDict();
  std::ifstream input(
      map_.at("serialization_settings"s).AsDict().at("file").AsString(),
      std::ios_base::in | std::ios_base::binary);
  transport_base::TransportCatalogue base;
  transpot_guide::TransportCatalogue transport_catalog;
  base.ParseFromIstream(&input);
  DerializeBusAndStop(transport_catalog, base);
  RenderSettings render_settings =
      DserializeRenderSettings(transport_catalog, base);
  graph::DirectedWeightedGraph<double> graph =
      DrserializeGraph(transport_catalog, base);
  DeserializeBtwStopInfo(transport_catalog, base);

  graph::Router router(graph);
  auto outputdat = json::Builder{}.StartArray().EndArray().Build().AsArray();

  for (auto& i : map_.at("stat_requests"s).AsArray()) {
    if (i.AsDict().at("type"s) == "Bus"s) {
      outputdat.push_back(transpot_guide::output::GetInfoRoute(
          transport_catalog, i.AsDict().at("name").AsString(),
          i.AsDict().at("id").AsInt()));
    }

    if (i.AsDict().at("type"s) == "Stop"s) {
      outputdat.push_back(transpot_guide::output::GetInfoStop(
          transport_catalog, i.AsDict().at("name").AsString(),
          i.AsDict().at("id").AsInt()));
    }

    if (i.AsDict().at("type") == "Map"s) {
      outputdat.push_back(GetMapOfRoad(transport_catalog, render_settings,
                                       i.AsDict().at("id").AsInt()));
    }

    if (i.AsDict().at("type"s) == "Route"s) {
      transpot_guide::Stop* from =
          transport_catalog.FindStop(i.AsDict().at("from"s).AsString());
      transpot_guide::Stop* to =
          transport_catalog.FindStop(i.AsDict().at("to"s).AsString());
      auto road = router.BuildRoute(from->id, to->id);
      if (road) {
        json::Array items;
        for (auto it : road.value().edges) {
          auto info = transport_catalog.GetWeith(it);
          if (!info.wait) {
            items.push_back(json::Builder{}
                                .StartDict()
                                .Key("type"s)
                                .Value("Wait"s)
                                .Key("stop_name"s)
                                .Value(std::string(info.stop_name))
                                .Key("time"s)
                                .Value(info.wait_time)
                                .EndDict()
                                .Build());

            items.push_back(json::Builder{}
                                .StartDict()
                                .Key("type"s)
                                .Value("Bus"s)
                                .Key("bus"s)
                                .Value(std::string(info.bus))
                                .Key("span_count"s)
                                .Value(info.span_count)
                                .Key("time"s)
                                .Value(info.travel_time)
                                .EndDict()
                                .Build());
          }
        }

        auto node = json::Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(i.AsDict().at("id").AsInt())
                        .Key("total_time"s)
                        .Value(road.value().weight)
                        .Key("items"s)
                        .Value(items)
                        .EndDict()
                        .Build();

        outputdat.push_back(node);
      } else {
        auto node = json::Builder{}
                        .StartDict()
                        .Key("request_id"s)
                        .Value(i.AsDict().at("id").AsInt())
                        .Key("error_message"s)
                        .Value("not found"s)
                        .EndDict()
                        .Build();

        outputdat.push_back(node);
      }
    }
  }

  return json::Node(outputdat);
}
