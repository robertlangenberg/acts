// This file is part of the Acts project.
//
// Copyright (C) 2016-2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include <cmath>

#include "Acts/Surfaces/DiscBounds.hpp"
#include "Acts/Utilities/Definitions.hpp"
#include "Acts/Utilities/ParameterDefinitions.hpp"

#include <array>
#include <vector>

namespace Acts {

/// @class DiscTrapezoidBounds
///
/// Class to describe the bounds for a planar DiscSurface.
/// By providing an argument for hphisec, the bounds can
/// be restricted to a phi-range around the center position.

class DiscTrapezoidBounds : public DiscBounds {
 public:
  enum BoundValues : int {
    eHalfLengthXminR = 0,
    eHalfLengthXmaxR = 1,
    eMinR = 2,
    eMaxR = 3,
    eAveragePhi = 4,
    eStereo = 5,
    eSize = 6
  };

  DiscTrapezoidBounds() = delete;

  /// Constructor for a symmetric Trapezoid giving min X length, max X length,
  /// Rmin and R max
  /// @param halfXminR half length in X at min radius
  /// @param halfXmaxR half length in X at maximum radius
  /// @param minR inner radius
  /// @param maxR outer radius
  /// @param avgPhi average phi value
  /// @param stereo optional stero angle applied
  DiscTrapezoidBounds(double halfXminR, double halfXmaxR, double minR,
                      double maxR, double avgPhi = M_PI_2, double stereo = 0.);

  /// Constructor - from fixed size array
  ///
  /// @param values The parameter values
  DiscTrapezoidBounds(const std::array<double, eSize>& values)
      : m_values(values) {}

  ~DiscTrapezoidBounds() override = default;

  DiscTrapezoidBounds* clone() const final;

  SurfaceBounds::BoundsType type() const final;

  /// Return the bound values as dynamically sized vector
  ///
  /// @return this returns a copy of the internal values
  std::vector<double> values() const final;

  ///  This method cheks if the radius given in the LocalPosition is inside
  ///  [rMin,rMax]
  /// if only tol0 is given and additional in the phi sector is tol1 is given
  /// @param lposition is the local position to be checked (in polar
  /// coordinates)
  /// @param bcheck is the boundary check directive
  bool inside(const Vector2D& lposition,
              const BoundaryCheck& bcheck = true) const final;

  /// Minimal distance to boundary
  /// @param lposition is the local position to be checked (in polar
  /// coordinates)
  /// @return is the minimal distance ( > 0 if outside and <=0 if inside)
  double distanceToBoundary(const Vector2D& lposition) const final;

  /// Output Method for std::ostream
  std::ostream& toStream(std::ostream& sl) const final;

  /// Access to the bound values
  /// @param bValue the class nested enum for the array access
  double get(BoundValues bValue) const { return m_values[bValue]; }

  /// This method returns inner radius
  double rMin() const final;

  /// This method returns outer radius
  double rMax() const final;

  /// This method returns the center radius
  double rCenter() const;

  /// This method returns the stereo angle
  double stereo() const;

  /// This method returns the halfPhiSector which is covered by the disc
  double halfPhiSector() const;

  /// This method returns the half length in Y (this is Rmax -Rmin)
  double halfLengthY() const;

  /// Returns true for full phi coverage - obviously false here
  bool coversFullAzimuth() const final;

  /// Checks if this is inside the radial coverage
  /// given the a tolerance
  bool insideRadialBounds(double R, double tolerance = 0.) const final;

  /// Return a reference radius for binning
  double binningValueR() const final;

  /// Return a reference phi for binning
  double binningValuePhi() const final;

  /// This method returns the xy coordinates of the four corners of the
  /// bounds in module coorindates (in xy)
  ///
  /// @param lseg the number of segments used to approximate
  /// and eventually curved line
  ///
  /// @note that the number of segments are ignored for this surface
  ///
  /// @return vector for vertices in 2D
  std::vector<Vector2D> vertices(unsigned int lseg) const;

 private:
  std::array<double, eSize> m_values;
  double m_stereo;  // TODO 2017-04-09 msmk: what is this good for?

  /// Private helper method to convert a local postion
  /// into its Cartesian representation
  ///
  /// @param lposition The local position in polar coordinates
  Vector2D toLocalCartesian(const Vector2D& lposition) const;

  /// Jacobian
  /// into its Cartesian representation
  ///
  /// @param lposition The local position in polar coordinates
  ActsMatrixD<2, 2> jacobianToLocalCartesian(const Vector2D& lposition) const;
};

inline double DiscTrapezoidBounds::rMin() const {
  return get(eMinR);
}

inline double DiscTrapezoidBounds::rMax() const {
  return get(eMaxR);
}

inline double DiscTrapezoidBounds::stereo() const {
  return m_stereo;
}

inline double DiscTrapezoidBounds::halfPhiSector() const {
  auto minHalfPhi = std::asin(get(eHalfLengthXminR) / get(eMinR));
  auto maxHalfPhi = std::asin(get(eHalfLengthXmaxR) / get(eMaxR));
  return std::max(minHalfPhi, maxHalfPhi);
}

inline double DiscTrapezoidBounds::rCenter() const {
  double rmin = get(eMinR);
  double rmax = get(eMaxR);
  double hxmin = get(eHalfLengthXminR);
  double hxmax = get(eHalfLengthXminR);
  auto hmin = std::sqrt(rmin * rmin - hxmin * hxmin);
  auto hmax = std::sqrt(rmax * rmax - hxmax * hxmax);
  return 0.5 * (hmin + hmax);
}

inline double DiscTrapezoidBounds::halfLengthY() const {
  double rmin = get(eMinR);
  double rmax = get(eMaxR);
  double hxmin = get(eHalfLengthXminR);
  double hxmax = get(eHalfLengthXminR);
  auto hmin = std::sqrt(rmin * rmin - hxmin * hxmin);
  auto hmax = std::sqrt(rmax * rmax - hxmax * hxmax);
  return 0.5 * (hmax - hmin);
}

inline bool DiscTrapezoidBounds::coversFullAzimuth() const {
  return false;
}

inline bool DiscTrapezoidBounds::insideRadialBounds(double R,
                                                    double tolerance) const {
  return (R + tolerance > get(eMinR) and R - tolerance < get(eMaxR));
}

inline double DiscTrapezoidBounds::binningValueR() const {
  return 0.5 * (get(eMinR) + get(eMaxR));
}

inline double DiscTrapezoidBounds::binningValuePhi() const {
  return get(eAveragePhi);
}

inline std::vector<double> DiscTrapezoidBounds::values() const {
  std::vector<double> valvector;
  valvector.insert(valvector.begin(), m_values.begin(), m_values.end());
  return valvector;
}

}  // namespace Acts