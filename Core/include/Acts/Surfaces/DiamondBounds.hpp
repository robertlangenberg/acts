// This file is part of the Acts project.
//
// Copyright (C) 2016-2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Surfaces/PlanarBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Utilities/Definitions.hpp"
#include "Acts/Utilities/ParameterDefinitions.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace Acts {

/// @class DiamondBounds
///
/// Bounds for a double trapezoidal ("diamond"), planar Surface.
class DiamondBounds : public PlanarBounds {
 public:
  enum BoundValues {
    eHalfLengthXnegY = 0,
    eHalfLengthXzeroY = 1,
    eHalfLengthXposY = 2,
    eHalfLengthYneg = 3,
    eHalfLengthYpos = 4,
    eSize = 5
  };

  DiamondBounds() = delete;

  /// Constructor for convex hexagon symmetric about the y axis
  ///
  /// @param halfXnegY is the halflength in x at minimal y
  /// @param halfXzeroY is the halflength in x at y = 0
  /// @param halfXposY is the halflength in x at maximal y
  /// @param halfYneg is the halflength into y < 0
  /// @param halfYpos is the halflength into y > 0
  DiamondBounds(double halfXnegY, double halfXzeroY, double halfXposY,
                double halfYneg, double halfYpos) noexcept(false)
      : m_values({halfXnegY, halfXzeroY, halfXposY, halfYneg, halfYpos}),
        m_boundingBox(*std::max_element(m_values.begin(), m_values.begin() + 2),
                      std::max(halfYneg, halfYpos)) {
    checkConsistency();
  }

  /// Constructor - from fixed size array
  ///
  /// @param values The parameter values
  DiamondBounds(const std::array<double, eSize>& values) noexcept(false)
      : m_values(values),
        m_boundingBox(
            *std::max_element(values.begin(), values.begin() + 2),
            std::max(values[eHalfLengthYneg], values[eHalfLengthYpos])) {}

  ~DiamondBounds() override = default;

  DiamondBounds* clone() const final;

  BoundsType type() const final;

  /// Return the bound values as dynamically sized vector
  ///
  /// @return this returns a copy of the internal values
  std::vector<double> values() const final;

  /// Inside check for the bounds object driven by the boundary check directive
  /// Each Bounds has a method inside, which checks if a LocalPosition is inside
  /// the bounds  Inside can be called without/with tolerances.
  ///
  /// @param lposition Local position (assumed to be in right surface frame)
  /// @param bcheck boundary check directive
  /// @return boolean indicator for the success of this operation
  bool inside(const Vector2D& lposition,
              const BoundaryCheck& bcheck) const final;

  /// Minimal distance to boundary ( > 0 if outside and <=0 if inside)
  ///
  /// @param lposition is the local position to check for the distance
  /// @return is a signed distance parameter
  double distanceToBoundary(const Vector2D& lposition) const final;

  /// Return the vertices
  ///
  /// @param lseg the number of segments used to approximate
  /// and eventually curved line
  ///
  /// @note the number of segements is ignored for this representation
  ///
  /// @return vector for vertices in 2D
  std::vector<Vector2D> vertices(unsigned int lseg = 1) const final;

  // Bounding box representation
  const RectangleBounds& boundingBox() const final;

  /// Output Method for std::ostream
  ///
  /// @param sl is the ostream in which it is dumped
  std::ostream& toStream(std::ostream& sl) const final;

  /// Access to the bound values
  /// @param bValue the class nested enum for the array access
  double get(BoundValues bValue) const { return m_values[bValue]; }

 private:
  std::array<double, eSize> m_values;
  RectangleBounds m_boundingBox;  ///< internal bounding box cache

  /// Check the input values for consistency, will throw a logic_exception
  /// if consistency is not given
  void checkConsistency() throw(std::logic_error);
};

inline std::vector<double> DiamondBounds::values() const {
  std::vector<double> valvector;
  valvector.insert(valvector.begin(), m_values.begin(), m_values.end());
  return valvector;
}

inline void DiamondBounds::checkConsistency() throw(std::logic_error) {
  if (std::any_of(m_values.begin(), m_values.end(),
                  [](auto v) { return v < 0.; })) {
    throw std::invalid_argument(
        "DiamondBounds: negative half length provided.");
  }
  if (get(eHalfLengthXnegY) > get(eHalfLengthXzeroY) or
      get(eHalfLengthXposY) > get(eHalfLengthXzeroY)) {
    throw std::invalid_argument("DiamondBounds: not a diamond shape.");
  }
}

}  // namespace Acts