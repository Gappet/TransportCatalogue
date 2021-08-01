#pragma once

#include <cmath>

namespace transpot_guide {
namespace detail {
struct Coordinates {
  double lat;
  double lng;
};

double ComputeDistance(Coordinates from, Coordinates to);
}  // namespace detail
}  // namespace transpot_guide
