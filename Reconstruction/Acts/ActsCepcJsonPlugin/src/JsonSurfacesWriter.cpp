// This file is part of the Acts project.
//
// Copyright (C) 2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "JsonSurfacesWriter.hpp"

#include "Acts/Geometry/GeometryHierarchyMap.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Plugins/Json/GeometryHierarchyMapJsonConverter.hpp"
#include "Acts/Plugins/Json/SurfaceJsonConverter.hpp"
#include "Acts/Surfaces/Surface.hpp"
//#include "ActsExamples/Utilities/Paths.hpp"


JsonSurfacesWriter::JsonSurfacesWriter(const JsonSurfacesWriter::Config& cfg,
                                       Acts::Logging::Level lvl)
    : m_cfg(cfg),
      m_world(nullptr),
      m_logger(Acts::getDefaultLogger("JsonSurfacesWriter", lvl)) {
  if (not m_cfg.trackingGeometry) {
    throw std::invalid_argument("Missing tracking geometry");
  }
  m_world = m_cfg.trackingGeometry->highestTrackingVolume();
  if (not m_world) {
    throw std::invalid_argument("Could not identify the world volume");
  }
}

std::string JsonSurfacesWriter::name() const {
  return "JsonSurfacesWriter";
}

namespace {

using SurfaceContainer =
    Acts::GeometryHierarchyMap<std::shared_ptr<const Acts::Surface>>;
using SurfaceConverter = Acts::GeometryHierarchyMapJsonConverter<
    std::shared_ptr<const Acts::Surface>>;

/// Write all child surfaces and descend into confined volumes.
void collectSurfaces(std::vector<SurfaceContainer::InputElement>& cSurfaces,
                     const Acts::TrackingVolume& volume, bool writeLayer,
                     bool writeApproach, bool writeSensitive,
                     bool writeBoundary) {
  // Process all layers that are directly stored within this volume
  if (volume.confinedLayers()) {
    for (auto layer : volume.confinedLayers()->arrayObjects()) {
      // We jump navigation layers
      if (layer->layerType() == Acts::navigation) {
        continue;
      }
      // Layer surface
      if (writeLayer) {
        auto layerSurfacePtr = layer->surfaceRepresentation().getSharedPtr();
        cSurfaces.push_back(SurfaceContainer::InputElement{
            layer->surfaceRepresentation().geometryId(), layerSurfacePtr});
      }
      // Approach surfaces
      if (writeApproach and layer->approachDescriptor()) {
        for (auto sf : layer->approachDescriptor()->containedSurfaces()) {
          cSurfaces.push_back(SurfaceContainer::InputElement{
              sf->geometryId(), sf->getSharedPtr()});
        }
      }
      // Check for sensitive surfaces
      if (layer->surfaceArray() and writeSensitive) {
        for (auto surface : layer->surfaceArray()->surfaces()) {
          if (surface) {
            cSurfaces.push_back(
                SurfaceContainer::InputElement{surface->geometryId(), surface});
          }
        }
      }
    }
    // This is a navigation volume, write the boundaries
    if (writeBoundary) {
      for (auto bsurface : volume.boundarySurfaces()) {
        const auto& bsRep = bsurface->surfaceRepresentation();
        cSurfaces.push_back(SurfaceContainer::InputElement{
            bsRep.geometryId(), bsRep.getSharedPtr()});
      }
    }
  }
  // Step down into hierarchy to process all child volumnes
  if (volume.confinedVolumes()) {
    for (auto confined : volume.confinedVolumes()->arrayObjects()) {
      collectSurfaces(cSurfaces, *confined.get(), writeLayer, writeApproach,
                      writeSensitive, writeBoundary);
    }
  }
}
}  // namespace

void JsonSurfacesWriter::write() {
  if (not m_cfg.writePerEvent) {
	return;
  }

  std::ofstream out;
  out.open("detector.json");

  std::vector<SurfaceContainer::InputElement> cSurfaces;
  collectSurfaces(cSurfaces, *m_world, m_cfg.writeLayer, m_cfg.writeApproach,
                  m_cfg.writeSensitive, m_cfg.writeBoundary);
  SurfaceContainer sContainer(cSurfaces);

  auto j = SurfaceConverter("surfaces").toJson(sContainer);
  out << std::setprecision(m_cfg.outputPrecision) << j.dump(2);
  out.close();
}
