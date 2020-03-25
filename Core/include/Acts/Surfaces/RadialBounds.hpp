// This file is part of the Acts project.
//
// Copyright (C) 2016-2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Surfaces/DiscBounds.hpp"
#include "Acts/Utilities/Definitions.hpp"
#include "Acts/Utilities/ParameterDefinitions.hpp"
#include "Acts/Utilities/detail/periodic.hpp"

#include <array>
#include <vector>

namespace Acts {

/// @class RadialBounds
///
/// Class to describe the bounds for a planar DiscSurface.
/// By providing an argument for hphisec, the bounds can
/// be restricted to a phi-range around the center position.
///
class RadialBounds : public DiscBounds {
 public:
  enum BoundValues {
    eMinR = 0,
    eMaxR = 1,
    eAveragePhi = 2,
    eHalfPhiSector = 3,
    eSize = 4
  };

  RadialBounds() = delete;

  /// Constructor for full disc of symmetric disc around phi=0
  ///
  /// @param minR The inner radius (0 for full disc)
  /// @param maxR The outer radius
  /// @param halfPhi The half opening angle (Pi for full angular coverage)
  /// @param avgPhi The average phi for the disc/ring sector
  RadialBounds(double minR, double maxR, double halfPhi = M_PI,
               double avgPhi = 0.)
      : m_values({std::abs(minR), std::abs(maxR), std::abs(halfPhi),
                  detail::radian_sym(avgPhi)}) {}

  /// Constructor from array values
  ///
  /// @param values The bound values
  RadialBounds(const std::array<double, eSize>& values) : m_values(values) {}

  ~RadialBounds() override = default;

  RadialBounds* clone() const final;

  SurfaceBounds::BoundsType type() const final;

  /// Return the bound values as dynamically sized vector
  ///
  /// @return this returns a copy of the internal values
  std::vector<double> values() const final;

  /// For disc surfaces the local position in (r,phi) is checked
  ///
  /// @param lposition local position to be checked
  /// @param bcheck boundary check directive
  ///
  /// @return is a boolean indicating the operation success
  bool inside(const Vector2D& lposition,
              const BoundaryCheck& bcheck) const final;

  /// Minimal distance to boundary calculation
  ///
  /// @param lposition local 2D position in surface coordinate frame
  ///
  /// @return distance to boundary ( > 0 if outside and <=0 if inside)
  double distanceToBoundary(const Vector2D& lposition) const final;

  /// Outstream operator
  ///
  /// @param sl is the ostream to be dumped into
  std::ostream& toStream(std::ostream& sl) const final;

  /// Return method for inner Radius
  double rMin() const;

  /// Return method for outer Radius
  double rMax() const;

  /// Access to the bound values
  /// @param bValue the class nested enum for the array access
  double get(BoundValues bValue) const { return m_values[bValue]; }

  /// Returns true for full phi coverage
  bool coversFullAzimuth() const final;

  /// Checks if this is inside the radial coverage
  /// given the a tolerance
  bool insideRadialBounds(double R, double tolerance = 0.) const final;

  /// Return a reference radius for binning
  double binningValueR() const final;

  /// Return a reference radius for binning
  double binningValuePhi() const final;

 private:
  std::array<double, eSize> m_values;

  /// Private helper method to shift a local position
  /// within the bounds
  ///
  /// @param lposition The local position in polar coordinates
  Vector2D shifted(const Vector2D& lposition) const;

  /// This method returns the xy coordinates of vertices along
  /// the radial bounds
  ///
  /// @param lseg the number of segments used to approximate
  /// and eventually curved line
  ///
  /// @note that the extremas are given, which may slightly alter the
  /// number of segments returned
  ///
  /// @return vector for vertices in 2D
  std::vector<Vector2D> vertices(unsigned int lseg) const;
};

inline double RadialBounds::rMin() const {
  return get(eMinR);
}

inline double RadialBounds::rMax() const {
  return get(eMaxR);
}

inline bool RadialBounds::coversFullAzimuth() const {
  return (get(eHalfPhiSector) == M_PI);
}

inline bool RadialBounds::insideRadialBounds(double R, double tolerance) const {
  return (R + tolerance > get(eMinR) and R - tolerance < get(eMaxR));
}

inline double RadialBounds::binningValueR() const {
  return 0.5 * (get(eMinR) + get(eMaxR));
}

inline double RadialBounds::binningValuePhi() const {
  return get(eAveragePhi);
}

inline std::vector<double> RadialBounds::values() const {
  std::vector<double> valvector;
  valvector.insert(valvector.begin(), m_values.begin(), m_values.end());
  return valvector;
}

}  // namespace Acts