// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// clang-format off
#define BOOST_TEST_MODULE ImpactPoint3dEstimator Tests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/output_test_stream.hpp>
// clang-format on

#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Tests/CommonHelpers/FloatComparisons.hpp"
#include "Acts/Utilities/Definitions.hpp"
#include "Acts/Utilities/Units.hpp"
#include "Acts/Vertexing/Vertex.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Vertexing/ImpactPoint3dEstimator.hpp"

namespace Acts {
namespace Test {

using Covariance = BoundSymMatrix;

// Create a test context
GeometryContext tgContext = GeometryContext();
MagneticFieldContext mfContext = MagneticFieldContext();

// Track d0 distribution
std::uniform_real_distribution<> d0Dist(-0.01 * units::_mm, 0.01 * units::_mm);
// Track z0 distribution
std::uniform_real_distribution<> z0Dist(-0.2 * units::_mm, 0.2 * units::_mm);
// Track pT distribution
std::uniform_real_distribution<> pTDist(0.4 * units::_GeV, 10. * units::_GeV);
// Track phi distribution
std::uniform_real_distribution<> phiDist(-M_PI, M_PI);
// Track theta distribution
std::uniform_real_distribution<> thetaDist(1.0, M_PI - 1.0);
// Track IP resolution distribution
std::uniform_real_distribution<> resIPDist(0., 100. * units::_um);
// Track angular distribution
std::uniform_real_distribution<> resAngDist(0., 0.1);
// Track q/p resolution distribution
std::uniform_real_distribution<> resQoPDist(-0.1, 0.1);
// Track charge helper distribution
std::uniform_real_distribution<> qDist(-1, 1);

/// @brief Unit test for ImpactPoint3dEstimator
///
BOOST_AUTO_TEST_CASE(impactpoint_3d_estimator_test) {
  // Debug mode
  bool debugMode = false;
  // Number of tests
  unsigned int nTests = 10;

  // Set up RNG
  int mySeed = 31415;
  std::mt19937 gen(mySeed);

  // Set up constant B-Field
  ConstantBField bField(Vector3D(0., 0., 1.) * units::_T);

  // Set up Eigenstepper
  EigenStepper<ConstantBField> stepper(bField);

  // Set up propagator with void navigator
  Propagator<EigenStepper<ConstantBField>> propagator(stepper);

  // Set up the ImpactPoint3dEstimator
  ImpactPoint3dEstimator<ConstantBField, BoundParameters,
                         Propagator<EigenStepper<ConstantBField>>>::Config
      ipEstCfg(bField, propagator);

  ImpactPoint3dEstimator<ConstantBField, BoundParameters,
                         Propagator<EigenStepper<ConstantBField>>>
      ipEstimator(ipEstCfg);

  // Reference position
  Vector3D refPosition(0., 0., 0.);

  // Start running tests
  for (unsigned int i = 0; i < nTests; i++) {
    // Create a track
    // Resolutions
    double resD0 = resIPDist(gen);
    double resZ0 = resIPDist(gen);
    double resPh = resAngDist(gen);
    double resTh = resAngDist(gen);
    double resQp = resQoPDist(gen);

    // Covariance matrix
    std::unique_ptr<Covariance> covMat = std::make_unique<Covariance>();
    (*covMat) << resD0 * resD0, 0., 0., 0., 0., 0., 0., resZ0 * resZ0, 0., 0.,
        0., 0., 0., 0., resPh * resPh, 0., 0., 0., 0., 0., 0., resTh * resTh,
        0., 0., 0., 0., 0., 0., resQp * resQp, 0., 0., 0., 0., 0., 0., 1.;

    // The charge
    double q = qDist(gen) < 0 ? -1. : 1.;

    // Impact parameters (IP)
    double d0 = d0Dist(gen);
    double z0 = z0Dist(gen);

    if (debugMode) {
      std::cout << "IP: (" << d0 << "," << z0 << ")" << std::endl;
    }

    // The track parameters
    TrackParametersBase::ParVector_t paramVec;
    paramVec << d0, z0, phiDist(gen), thetaDist(gen), q / pTDist(gen), 0.;

    // Corresponding surface
    std::shared_ptr<PerigeeSurface> perigeeSurface =
        Surface::makeShared<PerigeeSurface>(Vector3D(0., 0., 0.));

    // Creating the track
    BoundParameters myTrack(tgContext, std::move(covMat), paramVec,
                            perigeeSurface);

    // Distance in transverse plane
    double transverseDist = std::sqrt(std::pow(d0, 2) + std::pow(z0, 2));

    // Estimate 3D distance
    double distance = ipEstimator.calculateDistance(myTrack, refPosition);

    BOOST_CHECK(distance < transverseDist);

    if (debugMode) {
      std::cout << std::setprecision(10)
                << "Distance in transverse plane: " << transverseDist
                << std::endl;
      std::cout << std::setprecision(10) << "Distance in 3D: " << distance
                << std::endl;
    }

    auto res =
        ipEstimator.getParamsAtIP3d(tgContext, mfContext, myTrack, refPosition);

    BOOST_CHECK(res.ok());

    BoundParameters trackAtIP3d = std::move(**res);

    const auto& myTrackParams = myTrack.parameters();
    const auto& trackIP3dParams = trackAtIP3d.parameters();

    // d0 and z0 should have changed
    BOOST_CHECK_NE(myTrackParams[ParID_t::eLOC_D0],
                   trackIP3dParams[ParID_t::eLOC_D0]);
    BOOST_CHECK_NE(myTrackParams[ParID_t::eLOC_Z0],
                   trackIP3dParams[ParID_t::eLOC_Z0]);
    // Theta along helix and q/p shoud remain the same
    CHECK_CLOSE_REL(myTrackParams[ParID_t::eTHETA],
                    trackIP3dParams[ParID_t::eTHETA], 1e-5);
    CHECK_CLOSE_REL(myTrackParams[ParID_t::eQOP],
                    trackIP3dParams[ParID_t::eQOP], 1e-5);

    if (debugMode) {
      std::cout << std::setprecision(10) << "Old track parameters: \n"
                << myTrackParams << std::endl;
      std::cout << std::setprecision(10) << "Parameters at IP3d: \n"
                << trackIP3dParams << std::endl;
    }
  }  // end for loop tests
}

}  // namespace Test
}  // namespace Acts