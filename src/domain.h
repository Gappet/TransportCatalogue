#pragma once

#include <deque>
#include <string>
#include <tuple>
#include <utility>

#include "geo.h"

static const double precision = 1e-6;

namespace transpot_guide {

struct Stop {
  std::string name;
  detail::Coordinates coordinates;
  size_t id;

  std::tuple<double, double> AsTuple() const;

  bool operator==(const Stop& other) const {
    return (std::abs(coordinates.lat - other.coordinates.lat) < 1e-6) &&
           (std::abs(coordinates.lng - other.coordinates.lng) < 1e-6);
  }
};

struct PairStopHash {
  size_t operator()(std::pair<const Stop*, const Stop*> other) const {
    return hasher(other.first) + 1000 * hasher(other.second);
  }
  std::hash<const void*> hasher;
};

struct Bus {
  std::string name;
  std::deque<Stop*> route_stops;
  int stops_on_route = 0;
  int unique_stops = 0;
  double lengh = 0;
  double curvature = 1;
  bool is_roundtrip = false;
  bool operator==(const Bus other) const { return this->name == other.name; }
};
}  // namespace transpot_guide
