#pragma once

#include <transport_catalogue.pb.h>

#include <algorithm>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

using namespace std::string_literals;
static const std::string separator = "[:|||:]";

void SerializeBusAndStop(transpot_guide::TransportCatalogue& transport_catalog,
                         transport_base::TransportCatalogue& base);

void SerializeRenderSettings(
    transpot_guide::TransportCatalogue& transport_catalog,
    transport_base::TransportCatalogue& base, RenderSettings& settings);

void SerializeGraph(transpot_guide::TransportCatalogue& transport_catalog,
                    transport_base::TransportCatalogue& base);

void SerializeBtwStopInfo(transpot_guide::TransportCatalogue& transport_catalog,
                          transport_base::TransportCatalogue& base);

void MakeBase(transpot_guide::TransportCatalogue& transport_catalog,
              RenderSettings& settings, std::ostream& output);

void DerializeBusAndStop(transpot_guide::TransportCatalogue& transport_catalog,
                         transport_base::TransportCatalogue& base);

RenderSettings DserializeRenderSettings(
    transpot_guide::TransportCatalogue& transport_catalog,
    transport_base::TransportCatalogue& base);

void DeserializeBtwStopInfo(
    transpot_guide::TransportCatalogue& transport_catalog,
    transport_base::TransportCatalogue& base);

graph::DirectedWeightedGraph<double> DrserializeGraph(
    transpot_guide::TransportCatalogue& transport_catalog,
    transport_base::TransportCatalogue& base);

json::Node ProcessRequests(const json::Document& query);
