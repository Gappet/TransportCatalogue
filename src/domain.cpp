#include "domain.h"

namespace transpot_guide {
std::tuple<double, double> Stop::AsTuple() const {
  return std::make_tuple(coordinates.lat, coordinates.lng);
}
}  // namespace transpot_guide
