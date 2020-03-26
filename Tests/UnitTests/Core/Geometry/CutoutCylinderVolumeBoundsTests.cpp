// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/test/unit_test.hpp>

#include <fstream>
#include <iostream>
#include <memory>

#include "Acts/Geometry/CutoutCylinderVolumeBounds.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/Polyhedron.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Tests/CommonHelpers/FloatComparisons.hpp"
#include "Acts/Tests/CommonHelpers/ObjTestWriter.hpp"
#include "Acts/Utilities/Definitions.hpp"
#include "Acts/Utilities/PlyHelper.hpp"

namespace Acts {
namespace Test {

BOOST_AUTO_TEST_SUITE(Geometry)

BOOST_AUTO_TEST_CASE(CutoutCylinderVolumeBoundsConstruction) {
  CutoutCylinderVolumeBounds ccvb(5, 10, 15, 30, 25);
  ccvb.toStream(std::cout);

  // Test copy construction
  CutoutCylinderVolumeBounds copied(ccvb);
  BOOST_CHECK_EQUAL(ccvb, copied);

  // Test assigned
  CutoutCylinderVolumeBounds assigned = ccvb;
  BOOST_CHECK_EQUAL(ccvb, assigned);
}

BOOST_AUTO_TEST_CASE(CutoutCylinderVolumeBoundsRecreation) {
  CutoutCylinderVolumeBounds original(5, 10, 15, 30, 25);
  std::array<double, CutoutCylinderVolumeBounds::eSize> values;
  std::vector<double> valvector = original.values();
  std::copy_n(valvector.begin(), CutoutCylinderVolumeBounds::eSize,
              values.begin());
  CutoutCylinderVolumeBounds recreated(values);
  BOOST_CHECK_EQUAL(original, recreated);
}

BOOST_AUTO_TEST_CASE(CutoutCylinderVolumeBoundsExceptions) {
  double rmin{5}, rmed{10}, rmax{15}, hz{30}, hzc{25};

  // Test negative rmin
  BOOST_CHECK_THROW(CutoutCylinderVolumeBounds(-rmin, rmed, rmax, hz, hzc),
                    std::logic_error);

  // Test negative rmed
  BOOST_CHECK_THROW(CutoutCylinderVolumeBounds(rmin, -rmed, rmax, hz, hzc),
                    std::logic_error);

  // Test negative rmax
  BOOST_CHECK_THROW(CutoutCylinderVolumeBounds(rmin, rmed, -rmax, hz, hzc),
                    std::logic_error);

  // Test swapped rmin / rmed
  BOOST_CHECK_THROW(CutoutCylinderVolumeBounds(rmed, rmin, rmax, hz, hzc),
                    std::logic_error);

  // Test swapped rmin / rmax
  BOOST_CHECK_THROW(CutoutCylinderVolumeBounds(rmax, rmed, rmin, hz, hzc),
                    std::logic_error);

  // Test swapped rmed / rmax
  BOOST_CHECK_THROW(CutoutCylinderVolumeBounds(rmin, rmax, rmed, hz, hzc),
                    std::logic_error);

  // Test negative hz
  BOOST_CHECK_THROW(CutoutCylinderVolumeBounds(rmin, rmed, rmax, -hz, hzc),
                    std::logic_error);

  // Test negative hzc
  BOOST_CHECK_THROW(CutoutCylinderVolumeBounds(rmin, rmed, rmax, hz, -hzc),
                    std::logic_error);
}

BOOST_AUTO_TEST_CASE(CutoutCylinderVolumeBoundsAccess) {
  double rmin{5}, rmed{10}, rmax{15}, hz{30}, hzc{25};
  CutoutCylinderVolumeBounds ccvb(rmin, rmed, rmax, hz, hzc);

  BOOST_CHECK_EQUAL(ccvb.get(CutoutCylinderVolumeBounds::eMinR), rmin);
  BOOST_CHECK_EQUAL(ccvb.get(CutoutCylinderVolumeBounds::eMedR), rmed);
  BOOST_CHECK_EQUAL(ccvb.get(CutoutCylinderVolumeBounds::eMaxR), rmax);
  BOOST_CHECK_EQUAL(ccvb.get(CutoutCylinderVolumeBounds::eHalfLengthZ), hz);
  BOOST_CHECK_EQUAL(ccvb.get(CutoutCylinderVolumeBounds::eHalfLengthZcutout),
                    hzc);
}

BOOST_AUTO_TEST_CASE(CutoutCylinderVolumeBoundsInside) {
  CutoutCylinderVolumeBounds ccvb(5, 10, 15, 30, 25);

  BOOST_CHECK(!ccvb.inside({0, 0, 0}));
  BOOST_CHECK(!ccvb.inside({0, 3, 0}));
  BOOST_CHECK(!ccvb.inside({3, 0, 0}));
  BOOST_CHECK(!ccvb.inside({0, 7, 0}));
  BOOST_CHECK(!ccvb.inside({7, 0, 0}));
  BOOST_CHECK(ccvb.inside({0, 13, 0}));
  BOOST_CHECK(ccvb.inside({13, 0, 0}));
  BOOST_CHECK(!ccvb.inside({0, 17, 0}));
  BOOST_CHECK(!ccvb.inside({17, 0, 0}));

  // outside in z
  BOOST_CHECK(!ccvb.inside({0, 0, 35}));
  BOOST_CHECK(!ccvb.inside({0, 0, -35}));
  BOOST_CHECK(!ccvb.inside({0, 3, 35}));
  BOOST_CHECK(!ccvb.inside({0, 3, -35}));
  BOOST_CHECK(!ccvb.inside({3, 0, 35}));
  BOOST_CHECK(!ccvb.inside({3, 0, -35}));
  BOOST_CHECK(!ccvb.inside({0, 10, 35}));
  BOOST_CHECK(!ccvb.inside({0, 10, -35}));
  BOOST_CHECK(!ccvb.inside({10, 0, 35}));
  BOOST_CHECK(!ccvb.inside({10, 0, -35}));
  BOOST_CHECK(!ccvb.inside({0, 20, 35}));
  BOOST_CHECK(!ccvb.inside({0, 20, -35}));
  BOOST_CHECK(!ccvb.inside({20, 0, 35}));
  BOOST_CHECK(!ccvb.inside({20, 0, -35}));

  // in the choke point in z
  BOOST_CHECK(!ccvb.inside({0, 0, 27}));
  BOOST_CHECK(!ccvb.inside({0, 0, -27}));
  BOOST_CHECK(!ccvb.inside({0, 3, 27}));
  BOOST_CHECK(!ccvb.inside({0, 3, -27}));
  BOOST_CHECK(!ccvb.inside({3, 0, 27}));
  BOOST_CHECK(!ccvb.inside({3, 0, -27}));
  BOOST_CHECK(ccvb.inside({0, 7, 27}));
  BOOST_CHECK(ccvb.inside({0, 7, -27}));
  BOOST_CHECK(ccvb.inside({7, 0, 27}));
  BOOST_CHECK(ccvb.inside({7, 0, -27}));
  BOOST_CHECK(ccvb.inside({0, 13, 27}));
  BOOST_CHECK(ccvb.inside({0, 13, -27}));
  BOOST_CHECK(ccvb.inside({13, 0, 27}));
  BOOST_CHECK(ccvb.inside({13, 0, -27}));
  BOOST_CHECK(!ccvb.inside({0, 17, 27}));
  BOOST_CHECK(!ccvb.inside({0, 17, -27}));
  BOOST_CHECK(!ccvb.inside({17, 0, 27}));
  BOOST_CHECK(!ccvb.inside({17, 0, -27}));

  // right inside the choke point in z
  BOOST_CHECK(!ccvb.inside({0, 0, 23}));
  BOOST_CHECK(!ccvb.inside({0, 0, -23}));
  BOOST_CHECK(!ccvb.inside({0, 3, 23}));
  BOOST_CHECK(!ccvb.inside({0, 3, -23}));
  BOOST_CHECK(!ccvb.inside({3, 0, 23}));
  BOOST_CHECK(!ccvb.inside({3, 0, -23}));
  BOOST_CHECK(!ccvb.inside({0, 7, 23}));
  BOOST_CHECK(!ccvb.inside({0, 7, -23}));
  BOOST_CHECK(!ccvb.inside({7, 0, 23}));
  BOOST_CHECK(!ccvb.inside({7, 0, -23}));
  BOOST_CHECK(ccvb.inside({0, 13, 23}));
  BOOST_CHECK(ccvb.inside({0, 13, -23}));
  BOOST_CHECK(ccvb.inside({13, 0, 23}));
  BOOST_CHECK(ccvb.inside({13, 0, -23}));
  BOOST_CHECK(!ccvb.inside({0, 17, 23}));
  BOOST_CHECK(!ccvb.inside({0, 17, -23}));
  BOOST_CHECK(!ccvb.inside({17, 0, 23}));
  BOOST_CHECK(!ccvb.inside({17, 0, -23}));
}

BOOST_AUTO_TEST_CASE(CutoutCylinderVolumeBoundsBoundingBox) {
  GeometryContext tgContext = GeometryContext();
  std::vector<IdentifiedPolyderon> tPolyhedrons;

  auto combineAndDecompose = [&](const SurfacePtrVector& surfaces,
                                 const std::string& name) -> void {
    std::string writeBase = std::string("CutoutCylinderVolumeBounds") + name;

    Polyhedron phCombined;
    size_t is = 0;
    for (const auto& sf : surfaces) {
      Polyhedron phComponent = sf->polyhedronRepresentation(tgContext, 72);
      phCombined.merge(phComponent);
      tPolyhedrons.push_back(
          {writeBase + std::string("_comp_") + std::to_string(is++), false,
           phComponent});
    }
    tPolyhedrons.push_back({writeBase, false, phCombined});
  };

  CutoutCylinderVolumeBounds ccvb(5, 10, 15, 30, 25);
  auto box = ccvb.boundingBox();
  CHECK_CLOSE_ABS(box.min(), Vector3D(-15, -15, -30), 1e-6);
  CHECK_CLOSE_ABS(box.max(), Vector3D(15, 15, 30), 1e-6);

  auto ccvbSurfaces = ccvb.decomposeToSurfaces();
  combineAndDecompose(ccvbSurfaces, "");
  ObjTestWriter::writeObj("CutoutCylinderVolumeBounds_BB", box);
  ObjTestWriter::writeObj(tPolyhedrons);
}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace Test
}  // namespace Acts
