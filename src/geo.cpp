#include "geo.h"

#include <cmath>

namespace transpot_guide {
namespace detail {
double ComputeDistance(Coordinates from, Coordinates to) {
  using namespace std;
  static const double dr = 3.1415926535 / 180.;
  return acos(sin(from.lat * dr) * sin(to.lat * dr) +
              cos(from.lat * dr) * cos(to.lat * dr) *
                  cos(abs(from.lng - to.lng) * dr)) *
         6371000;
}
}  // namespace detail
}  // namespace transpot_guide
