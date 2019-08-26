// This file is part of the Acts project.
//
// Copyright (C) 2016-2018 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include "Acts/EventData/ChargePolicy.hpp"
#include "Acts/EventData/SingleBoundTrackParameters.hpp"
#include "Acts/EventData/SingleCurvilinearTrackParameters.hpp"
#include "Acts/EventData/SingleTrackParameters.hpp"
#include "Acts/EventData/MultiTrackParameters.hpp"

namespace Acts {
using TrackParameters = SingleTrackParameters<ChargedPolicy>;
using CurvilinearParameters = SingleCurvilinearTrackParameters<ChargedPolicy>;
using BoundParameters = SingleBoundTrackParameters<ChargedPolicy>;
template<typename T>
    using MultipleTrackParameters = MultiTrackParameters<ChargedPolicy, T>;
}  // namespace Acts
