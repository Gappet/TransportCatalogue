#include "transport_catalogue.h"

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_set>
#include <utility>

using namespace std::string_literals;

namespace transpot_guide {
void TransportCatalogue::AddStop(std::string stop_name,
                                 detail::Coordinates coordinates, size_t id) {
  stops_.push_back(
      {std::move(stop_name), {coordinates.lat, coordinates.lng}, id});
  stop_coordinate_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddDistance(std::string_view stop_from,
                                     std::string_view stop_to, int dist) {
  Stop* from = FindStop(stop_from);
  Stop* to = FindStop(stop_to);
  lengh_btw_stop_[std::make_pair(from, to)] = dist;
}

void TransportCatalogue::AddRoute(std::string bus,
                                  std::vector<std::string_view> stops,
                                  bool is_roundtrip) {
  std::deque<Stop*> stops_ref;
  for (const auto& stop : stops) {
    stops_ref.push_back(FindStop(stop));
  }

  int stops_on_route;
  if (is_roundtrip) {
    stops_on_route = stops.size();
  } else {
    stops_on_route = 2 * stops.size() - 1;
  }
  int unique_stops =
      std::unordered_set<std::string_view>{stops.begin(), stops.end()}.size();

  double direct_lengh = 0;

  for (auto itr = stops_ref.begin(); itr < stops_ref.end() - 1; ++itr) {
    Stop* from = *itr;
    Stop* to = *(itr + 1);
    direct_lengh += ComputeDistance(from->coordinates, to->coordinates);
  }

  if (!is_roundtrip) {
    for (auto itr = stops_ref.rbegin(); itr < stops_ref.rend() - 1; ++itr) {
      Stop* from = *itr;
      Stop* to = *(itr + 1);
      direct_lengh += ComputeDistance(from->coordinates, to->coordinates);
    }
  }

  double real_lengh = 0;
  for (auto itr = stops_ref.begin(); itr < stops_ref.end() - 1; ++itr) {
    Stop* from = *itr;
    Stop* to = *(itr + 1);
    real_lengh += GetDistanceBetweenStop(from, to);
  }

  if (!is_roundtrip) {
    for (auto itr = stops_ref.rbegin(); itr < stops_ref.rend() - 1; ++itr) {
      Stop* from = *itr;
      Stop* to = *(itr + 1);
      real_lengh += GetDistanceBetweenStop(from, to);
    }
  }

  buses_.push_back({std::move(bus), std::move(stops_ref),
                    std::move(stops_on_route), std::move(unique_stops),
                    std::move(real_lengh), real_lengh / direct_lengh,
                    is_roundtrip});
  routes_[buses_.back().name] = &buses_.back();
  for (Stop* stop : buses_.back().route_stops) {
    buses_of_stop_[stop->name].insert(buses_.back().name);
  }
}

Bus* TransportCatalogue::FindRoute(std::string_view bus) {
  return routes_[bus];
}

Stop* TransportCatalogue::FindStop(std::string_view stop) {
  return stop_coordinate_[stop];
}

std::set<std::string_view> TransportCatalogue::GetBusofStop(
    std::string_view stop) {
  return buses_of_stop_[stop];
}

bool TransportCatalogue::IsBus(std::string_view bus) const {
  return routes_.count(bus);
}

bool TransportCatalogue::IsStop(std::string_view stop) const {
  return stop_coordinate_.count(stop);
}

std::deque<Stop*> TransportCatalogue::GetStops() {
  std::deque<Stop*> stops;
  for (auto& [first, second] : stop_coordinate_) {
    stops.push_back(second);
  }
  return stops;
}

std::deque<Bus*> TransportCatalogue::GetBus() {
  std::deque<Bus*> buses;
  for (auto& [first, second] : routes_) {
    if (!second->route_stops.empty()) {
      buses.push_back(second);
    }
  }
  return buses;
}

double TransportCatalogue::GetDistanceBetweenStop(Stop* stop_from,
                                                  Stop* stop_to) {
  double lenght = 0;
  if (lengh_btw_stop_.count(std::make_pair(stop_from, stop_to))) {
    lenght = double(lengh_btw_stop_[std::make_pair(stop_from, stop_to)]);
  } else if (lengh_btw_stop_.count(std::make_pair(stop_to, stop_from))) {
    lenght = double(lengh_btw_stop_[std::make_pair(stop_to, stop_from)]);
  } else {
    lenght = ComputeDistance(stop_from->coordinates, stop_to->coordinates);
  }
  return lenght;
}

void TransportCatalogue::SetBusVelocity(BusVelocity bus_velocity) {
  bus_velocity_ = (bus_velocity * 1000) / 60;
}

void TransportCatalogue::SetBusWaitTime(TimeMin bus_wait_time) {
  bus_wait_time_ = bus_wait_time;
}

graph::DirectedWeightedGraph<double> TransportCatalogue::BuildGraph() {
  graph::DirectedWeightedGraph<double> graph(stops_.size() * 2);

  for (auto bus = buses_.begin(); bus != buses_.end(); ++bus) {
    if (bus->is_roundtrip) {
      for (int stop_from = 0;
           stop_from < static_cast<int>(bus->route_stops.size()); ++stop_from) {
        Stop* stop_from_ref = bus->route_stops[stop_from];
        size_t id_from = stop_from_ref->id;
        graph.AddEdge(
            {id_from, stop_from_ref->id + stops_.size(), bus_wait_time_});
        int span_count = 1;
        double time = 0.0;
        for (int stop_to = stop_from + 1;
             stop_to < static_cast<int>(bus->route_stops.size()); ++stop_to) {
          Stop* stop_to_ref = bus->route_stops[stop_to];
          time += GetDistanceBetweenStop(stop_from_ref, stop_to_ref) /
                  bus_velocity_;
          stop_from_ref = stop_to_ref;
          graph::EdgeId id =
              graph.AddEdge({id_from + stops_.size(), stop_to_ref->id, time});
          btw_stop_info_.insert(
              {id,
               {bus_wait_time_, time, span_count,
                bus->route_stops[stop_from]->name, bus->name, false}});
          ++span_count;
        }
      }
    } else {
      for (int stop_from = 0;
           stop_from < static_cast<int>(bus->route_stops.size()); ++stop_from) {
        Stop* stop_from_ref = bus->route_stops[stop_from];
        size_t id_from = stop_from_ref->id;
        graph.AddEdge({id_from, id_from + stops_.size(), bus_wait_time_});
        double time = 0.0;
        int span_count = 1;
        for (int stop_to = stop_from + 1;
             stop_to < static_cast<int>(bus->route_stops.size()); ++stop_to) {
          Stop* stop_to_ref = bus->route_stops[stop_to];
          time += GetDistanceBetweenStop(stop_from_ref, stop_to_ref) /
                  bus_velocity_;
          stop_from_ref = stop_to_ref;
          graph::EdgeId id =
              graph.AddEdge({id_from + stops_.size(), stop_to_ref->id, time});
          btw_stop_info_.insert(
              {id,
               {bus_wait_time_, time, span_count,
                bus->route_stops[stop_from]->name, bus->name, false}});
          ++span_count;
        }
      }
      for (int stop_from = static_cast<int>(bus->route_stops.size()) - 1;
           stop_from != -1; --stop_from) {
        Stop* stop_from_ref = bus->route_stops[stop_from];
        size_t id_from = stop_from_ref->id;
        double time = 0.0;
        int span_count = 1;
        for (int stop_to = stop_from - 1; stop_to != -1; --stop_to) {
          Stop* stop_to_ref = bus->route_stops[stop_to];
          time += GetDistanceBetweenStop(stop_from_ref, stop_to_ref) /
                  bus_velocity_;
          stop_from_ref = stop_to_ref;
          graph::EdgeId id =
              graph.AddEdge({id_from + stops_.size(), stop_to_ref->id, time});
          btw_stop_info_.insert(
              {id,
               {bus_wait_time_, time, span_count,
                bus->route_stops[stop_from]->name, bus->name, false}});
          ++span_count;
        }
      }
    }
  }

  return graph;
}

TransportCatalogue::Weight TransportCatalogue::GetWeith(graph::EdgeId id) {
  return btw_stop_info_[id];
}

std::unordered_map<std::pair<Stop*, Stop*>, int, PairStopHash>&
TransportCatalogue::GetLenghBtwStop() {
  return lengh_btw_stop_;
}

std::map<graph::EdgeId, TransportCatalogue::Weight>&
TransportCatalogue::GetBtwStopInfo() {
  return btw_stop_info_;
}

void TransportCatalogue::SetBtwStopInfo(
    std::map<graph::EdgeId, TransportCatalogue::Weight> info) {
  btw_stop_info_ = info;
}

}  // namespace transpot_guide
