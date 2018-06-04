// This file is part of the Acts project.
//
// Copyright (C) 2018 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <vector>

#include "Acts/Utilities/Definitions.hpp"

namespace Acts {

struct PolyhedronRepresentation
{

  PolyhedronRepresentation(std::vector<Vector3D>            _vertices,
                           std::vector<std::vector<size_t>> _faces)
    : vertices(_vertices), faces(_faces)
  {
  }

  // list of 3D vertices as vectors
  std::vector<Vector3D> vertices;
  // list of faces connecting the vertices.
  // each face is a list of vertex indices
  // corresponding to the vertex vector above
  std::vector<std::vector<size_t>> faces;

  std::string
  objString(size_t vtxOffset = 0) const;
};
}
