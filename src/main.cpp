#include <iostream>

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
using namespace std;

int main() {
  transpot_guide::TransportCatalogue transport_catologue;
  json::Document jsons = json::Load(std::cin);
  auto map_ = jsons.GetRoot().AsDict();

  ::transpot_guide::input::InputData(transport_catologue,
                                     map_["base_requests"s].AsArray());
  RenderSettings settings;
  if (map_.count("render_settings"s)) {
    settings = ::transpot_guide::input::ReadRenderSettings(
        map_["render_settings"s].AsDict());
  }

  json::Document a(transpot_guide::output::OutputData(
      transport_catologue, map_["stat_requests"].AsArray(), settings));

  json::Print(a, cout);
  cout << endl;
}
