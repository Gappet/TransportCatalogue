#pragma once

#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "graph.h"

namespace transpot_guide {
using BusVelocity = double;  // km/h
using TimeMin = double;

class TransportCatalogue {
 public:
  struct Weight {
    TimeMin wait_time;
    TimeMin travel_time;
    int span_count;
    std::string_view stop_name;
    std::string_view bus;
    bool wait = true;
  };
  void AddStop(std::string stop_name, detail::Coordinates coordinates,
               size_t id);

  void AddRoute(std::string bus, std::vector<std::string_view> stops,
                bool is_roundtrip);

  void AddDistance(std::string_view stop_from, std::string_view stop_to,
                   int dist);

  void SetBusVelocity(BusVelocity bus_velocity);

  void SetBusWaitTime(TimeMin bus_wait_time);

  Bus* FindRoute(std::string_view state);

  Stop* FindStop(std::string_view state);

  bool IsBus(std::string_view bus) const;

  bool IsStop(std::string_view stop) const;

  std::set<std::string_view> GetBusofStop(std::string_view stop);

  std::deque<Stop*> GetStops();

  std::deque<Bus*> GetBus();

  graph::DirectedWeightedGraph<double> BuildGraph();

  graph::DirectedWeightedGraph<double>& GetGraph();

  Weight GetWeith(graph::EdgeId id);

  std::unordered_map<std::pair<Stop*, Stop*>, int, PairStopHash>&
  GetLenghBtwStop();

  std::map<graph::EdgeId, Weight>& GetBtwStopInfo();

  void SetBtwStopInfo(std::map<graph::EdgeId, Weight> info);

 private:
  double GetDistanceBetweenStop(Stop* stop_from, Stop* stop_to);

  std::deque<Stop> stops_;
  std::deque<Bus> buses_;
  std::unordered_map<std::string_view, Bus*> routes_;
  std::unordered_map<std::string_view, Stop*> stop_coordinate_;
  std::unordered_map<std::string_view, std::set<std::string_view>>
      buses_of_stop_;
  std::unordered_map<std::pair<Stop*, Stop*>, int, PairStopHash>
      lengh_btw_stop_;

  BusVelocity bus_velocity_;
  TimeMin bus_wait_time_;
  std::map<graph::EdgeId, Weight> btw_stop_info_;
};
}  // namespace transpot_guide
