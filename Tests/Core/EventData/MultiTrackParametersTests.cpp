// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// clang-format off
#define BOOST_TEST_MODULE MultiCurvilinearParameters Tests
#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
// clang-format on

#include "Acts/EventData/NeutralParameters.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Surfaces/CylinderBounds.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RadialBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Tests/CommonHelpers/FloatComparisons.hpp"
#include "Acts/Utilities/Definitions.hpp"
#include "Acts/Utilities/Units.hpp"

#include "ParametersTestHelper.hpp"
#include "Acts/EventData/MultiTrackParameters.hpp"

namespace bdata = boost::unit_test::data;
namespace tt    = boost::test_tools;

namespace Acts {
using VectorHelpers::phi;
using VectorHelpers::theta;
namespace Test {

  // Create a test context
  GeometryContext tgContext = GeometryContext();

  /// @brief Unit test for Curvilinear parameters
  ///
  BOOST_AUTO_TEST_CASE(multi_curvilinear_initialization)
  {
  using namespace Acts::UnitLiterals;

    // some position and momentum
    Vector3D pos0(1._mm, 2._mm, 3._mm);
    Vector3D pos1(2.01_mm, 2.01_mm, 3.01_mm);
    Vector3D pos2(3.02_mm, 2.02_mm, 3.02_mm);
    Vector3D mom0(1000._GeV, 1000._GeV, -0.100_GeV);
    Vector3D mom1(1000.01_GeV, 1000._GeV, -0.100_GeV);
    Vector3D mom2(1000.02_GeV, 1000._GeV, -0.100_GeV);
    Vector3D dir0(mom0.normalized());
    Vector3D dir1(mom1.normalized());
    Vector3D dir2(mom2.normalized());
	Vector3D dir_combine = (0.1 * mom0 + 0.6 * mom1 + 0.3 * mom2).normalized();
	Vector3D mom_combine = 0.1 * mom0 + 0.6 * mom1 + 0.3 * mom2;
	Vector3D pos_combine = 0.1 * pos0 + 0.6 * pos1 + 0.3 * pos2;
    Vector3D z_axis_global(0., 0., 1.);
    /// create curvilinear parameters without covariance +1/-1 charge
	
	CurvilinearParameters curvilinear_pos_0(std::nullopt, pos0, mom0, 1_e, 1_s);
	CurvilinearParameters curvilinear_pos_1(std::nullopt, pos1, mom1, 1_e, 1_s);
	CurvilinearParameters curvilinear_pos_2(std::nullopt, pos2, mom2, 1_e, 1_s);

    MultipleTrackParameters<CurvilinearParameters> multi_curvilinear_pos({{0.1,std::move(curvilinear_pos_0)},{0.6, std::move(curvilinear_pos_1)}});
    multi_curvilinear_pos.append(0.3, std::move(curvilinear_pos_2));
    BOOST_CHECK_EQUAL(multi_curvilinear_pos.size(), 3);
	
    // test sort in the trackMap 
    //MultipleTrackParameters<CurvilinearParameters>::ParameterMapWeightTrack::const_iterator it
    auto it = multi_curvilinear_pos.getTrackList().begin();
    BOOST_CHECK_EQUAL((*it).first, 0.6);
    ++it;
    BOOST_CHECK_EQUAL((*it).first, 0.3);
    ++it;
    BOOST_CHECK_EQUAL((*it).first, 0.1);

	// test pos/mom 
	CHECK_CLOSE_REL(multi_curvilinear_pos.position(), pos_combine, 1e-6);
	CHECK_CLOSE_REL(multi_curvilinear_pos.momentum(), mom_combine, 1e-6);

  // check that the created surface is at the position
  CHECK_CLOSE_REL(multi_curvilinear_pos.referenceSurface().center(tgContext), pos_combine,
                  1e-6);

  // check that the z-axis of the created surface is along momentum direction
  CHECK_CLOSE_REL(multi_curvilinear_pos.referenceSurface().normal(tgContext, pos_combine),
                  dir_combine, 1e-6);

	// check the reference frame of curvilinear parameters
	// it is the x-y frame of the created surface
	RotationMatrix3D mFrame = RotationMatrix3D::Zero();
	Vector3D         tAxis  = dir_combine;
	Vector3D         uAxis  = (z_axis_global.cross(tAxis)).normalized();
	Vector3D         vAxis  = tAxis.cross(uAxis);
	mFrame.col(0)           = uAxis;
	mFrame.col(1)           = vAxis;
	mFrame.col(2)           = tAxis;
	CHECK_CLOSE_OR_SMALL(mFrame, multi_curvilinear_pos.referenceFrame(tgContext), 1e-6, 1e-6);
  }  //BOOST Test for MultiCurvileaner

  /// @brief Unit test for parameters at a plane
  ///
  BOOST_DATA_TEST_CASE(
      bound_to_plane_test,
      bdata::random((bdata::seed = 1240,
                     bdata::distribution
                     = std::uniform_real_distribution<>(-1000., 1000.)))
          ^ bdata::random((bdata::seed = 2351,
                           bdata::distribution
                           = std::uniform_real_distribution<>(-1000., 1000.)))
          ^ bdata::random((bdata::seed = 3412,
                           bdata::distribution
                           = std::uniform_real_distribution<>(-1000., 1000.)))
          ^ bdata::random((bdata::seed = 5732,
                           bdata::distribution
                           = std::uniform_real_distribution<>(0., M_PI)))
          ^ bdata::random((bdata::seed = 8941,
                           bdata::distribution
                           = std::uniform_real_distribution<>(0., M_PI)))
          ^ bdata::random((bdata::seed = 1295,
                           bdata::distribution
                           = std::uniform_real_distribution<>(0., M_PI)))
          ^ bdata::xrange(1),
      x,
      y,
      z,
      a,
      b,
      c,
      index)
  {
	using namespace Acts::UnitLiterals;
    (void)index;
    auto     transform = std::make_shared<Transform3D>();
    transform->setIdentity();
    RotationMatrix3D rot;
    rot = AngleAxis3D(a, Vector3D::UnitX()) * AngleAxis3D(b, Vector3D::UnitY())
        * AngleAxis3D(c, Vector3D::UnitZ());
    Vector3D center{x, y, z};
    transform->prerotate(rot);
    transform->pretranslate(center);
    // create the surfacex
    auto bounds   = std::make_shared<RectangleBounds>(100., 100.);
    auto pSurface = Surface::makeShared<PlaneSurface>(
        transform, bounds);  // surface use +1

    // now create parameters on this surface
    // l_x, l_y, phi, theta, q/p (1/p)
    std::array<double, 6> pars_array = {{-0.1234, 9.8765, 0.45, 0.888, 0.001, 21.}};
    TrackParametersBase::ParVector_t pars;
    pars << pars_array[0], pars_array[1], pars_array[2], pars_array[3],
        pars_array[4], pars_array[5];

	BoundParameters ataPlane_from_pars_0(tgContext, std::nullopt, pars, pSurface);  //+2
	BoundParameters ataPlane_from_pars_1(tgContext, std::nullopt, pars, pSurface);  //+3

    // make multi bound par
    MultipleTrackParameters<BoundParameters> multi_ataPlane_from_pars({{0.3, std::move(ataPlane_from_pars_0)}}, pSurface);  //+4
    multi_ataPlane_from_pars.append(0.7, std::move(ataPlane_from_pars_1));

    // test the append method
    //MultipleTrackParameters<BoundParameters>::ParameterMapWeightTrack::const_iterator it
    auto it = multi_ataPlane_from_pars.getTrackList().begin();
    BOOST_CHECK_EQUAL((*it).first, 0.7);
    ++it;
    BOOST_CHECK_EQUAL((*it).first, 0.3);

    // check shared ownership of same surface
    BOOST_CHECK_EQUAL(&multi_ataPlane_from_pars.referenceSurface(),
                      pSurface.get());
    BOOST_CHECK_EQUAL(pSurface.use_count(), 4);

    // check that the reference frame is the rotation matrix
    CHECK_CLOSE_REL(
        multi_ataPlane_from_pars.referenceFrame(tgContext), rot, 1e-6);
  }
}  // namespace Test
}  // namespace Acts