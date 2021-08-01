#include "request_handler.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "geo.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_catalogue.h"

using namespace std::string_literals;

namespace transpot_guide {
namespace output {
json::Node GetInfoRoute(::transpot_guide::TransportCatalogue& transport_catalog,
                        const std::string_view bus, int id) {
  auto InfoRoute = json::Builder{}
                       .StartDict()
                       .Key("request_id"s)
                       .Value(id)
                       .EndDict()
                       .Build()
                       .AsDict();
  if (!transport_catalog.IsBus(bus)) {
    InfoRoute.insert({"error_message"s, json::Node("not found"s)});

  } else {
    Bus* route = transport_catalog.FindRoute(bus);

    InfoRoute.insert({"stop_count"s, json::Node(route->stops_on_route)});
    InfoRoute.insert({"unique_stop_count"s, json::Node(route->unique_stops)});
    InfoRoute.insert({"curvature"s, json::Node(route->curvature)});
    InfoRoute.insert({"route_length", json::Node(route->lengh)});
  }
  return json::Node(InfoRoute);
}

json::Node GetInfoStop(::transpot_guide::TransportCatalogue& transport_catalog,
                       const std::string_view stop, int id) {
  auto InfoStop = json::Builder{}
                      .StartDict()
                      .Key("request_id"s)
                      .Value(id)
                      .EndDict()
                      .Build()
                      .AsDict();
  if (!transport_catalog.IsStop(stop)) {
    InfoStop.insert({"error_message"s, json::Node("not found"s)});
  } else {
    auto buses = transport_catalog.GetBusofStop(stop);
    if (!buses.empty()) {
      auto buses_out =
          json::Builder{}.StartArray().EndArray().Build().AsArray();
      for (auto& bus : buses) {
        buses_out.push_back(json::Node(std::string(bus)));
      }
      InfoStop.insert({"buses"s, json::Node(buses_out)});

    } else {
      InfoStop.insert({"buses"s, json::Node(json::Array())});
    }
  }

  return json::Node(InfoStop);
}

json::Node OutputData(TransportCatalogue& transport_catalog, json::Array query,
                      RenderSettings& setting) {
  auto outputdat = json::Builder{}.StartArray().EndArray().Build().AsArray();
  graph::DirectedWeightedGraph graph_transpot = transport_catalog.BuildGraph();
  graph::Router router(graph_transpot);
  for (auto& i : query) {
    if (i.AsDict().at("type"s) == "Bus"s) {
      outputdat.push_back(GetInfoRoute(transport_catalog,
                                       i.AsDict().at("name").AsString(),
                                       i.AsDict().at("id").AsInt()));
    }
    if (i.AsDict().at("type"s) == "Map"s) {
      outputdat.push_back(GetMapOfRoad(transport_catalog, setting,
                                       i.AsDict().at("id").AsInt()));
    }

    if (i.AsDict().at("type"s) == "Stop"s) {
      outputdat.push_back(GetInfoStop(transport_catalog,
                                      i.AsDict().at("name").AsString(),
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
}  // namespace output
}  // namespace transpot_guide
