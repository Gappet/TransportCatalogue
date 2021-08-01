#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace transpot_guide {
namespace input {

void InputData(::transpot_guide::TransportCatalogue& transport_catalog,
               json::Array data);

svg::Color ParsingColor(const json::Node& color);

RenderSettings ReadRenderSettings(json::Dict settings);
}  // namespace input
}  // namespace transpot_guide
