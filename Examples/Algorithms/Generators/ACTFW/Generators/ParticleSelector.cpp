// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ACTFW/Generators/ParticleSelector.hpp"

#include "ACTFW/EventData/SimVertex.hpp"
#include "ACTFW/Framework/WhiteBoard.hpp"
#include "ACTFW/Utilities/Options.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Units.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <boost/program_options.hpp>

void FW::ParticleSelector::addOptions(FW::Options::Description& desc) {
  using boost::program_options::bool_switch;
  using boost::program_options::value;
  using Options::Interval;

  auto opt = desc.add_options();
  opt("select-rho-mm", value<Interval>()->value_name("MIN:MAX"),
      "Select particle transverse distance to the origin in mm");
  opt("select-absz-mm", value<Interval>()->value_name("MIN:MAX"),
      "Select particle absolute longitudinal distance to the origin in mm");
  opt("select-phi-degree", value<Interval>()->value_name("MIN:MAX"),
      "Select particle direction angle in the transverse plane in degree");
  opt("select-eta", value<Interval>()->value_name("MIN:MAX"),
      "Select particle pseudo-rapidity");
  opt("select-abseta", value<Interval>()->value_name("MIN:MAX"),
      "Select particle absolute pseudo-rapidity");
  opt("select-pt-gev", value<Interval>()->value_name("MIN:MAX"),
      "Select particle transverse momentum in GeV");
  opt("remove-charged", bool_switch(), "Remove charged particles");
  opt("remove-neutral", bool_switch(), "Remove neutral particles");
}

FW::ParticleSelector::Config FW::ParticleSelector::readConfig(
    const FW::Options::Variables& vars) {
  using namespace Acts::UnitLiterals;

  // Set boundary values if the given config exists
  auto extractInterval = [&](const char* name, auto unit, auto& lower,
                             auto& upper) {
    if (vars[name].empty()) {
      return;
    }
    auto interval = vars[name].as<Options::Interval>();
    lower = interval.lower.value_or(lower) * unit;
    upper = interval.upper.value_or(upper) * unit;
  };

  Config cfg;
  extractInterval("select-rho-mm", 1_mm, cfg.rhoMin, cfg.rhoMax);
  extractInterval("select-absz-mm", 1_mm, cfg.absZMin, cfg.absZMax);
  extractInterval("select-phi-degree", 1_degree, cfg.phiMin, cfg.phiMax);
  extractInterval("select-eta", 1.0, cfg.etaMin, cfg.etaMax);
  extractInterval("select-abseta", 1.0, cfg.absEtaMin, cfg.absEtaMax);
  extractInterval("select-pt-gev", 1_GeV, cfg.ptMin, cfg.ptMax);
  cfg.removeCharged = vars["remove-charged"].as<bool>();
  cfg.removeNeutral = vars["remove-neutral"].as<bool>();
  return cfg;
}

FW::ParticleSelector::ParticleSelector(const Config& cfg,
                                       Acts::Logging::Level lvl)
    : FW::BareAlgorithm("ParticleSelector", lvl), m_cfg(cfg) {
  if (m_cfg.inputEvent.empty()) {
    throw std::invalid_argument("Missing input event collection");
  }
  if (m_cfg.outputEvent.empty()) {
    throw std::invalid_argument("Missing output event collection");
  }
  ACTS_DEBUG("selection particle rho [" << m_cfg.rhoMin << "," << m_cfg.rhoMax
                                        << "]");
  ACTS_DEBUG("selection particle |z| [" << m_cfg.absZMin << "," << m_cfg.absZMax
                                        << "]");
  ACTS_DEBUG("selection particle phi [" << m_cfg.phiMin << "," << m_cfg.phiMax
                                        << "]");
  ACTS_DEBUG("selection particle eta [" << m_cfg.etaMin << "," << m_cfg.etaMax
                                        << "]");
  ACTS_DEBUG("selection particle |eta| [" << m_cfg.absEtaMin << ","
                                          << m_cfg.absEtaMax << "]");
  ACTS_DEBUG("selection particle pt [" << m_cfg.ptMin << "," << m_cfg.ptMax
                                       << "]");
  ACTS_DEBUG("remove charged particles " << m_cfg.removeCharged);
  ACTS_DEBUG("remove neutral particles " << m_cfg.removeNeutral);
}

FW::ProcessCode FW::ParticleSelector::execute(
    const FW::AlgorithmContext& ctx) const {
  using SimEvent = std::vector<SimVertex>;

  // prepare input/ output types
  const auto& input = ctx.eventStore.get<SimEvent>(m_cfg.inputEvent);
  SimEvent selected;

  auto within = [](double x, double min, double max) {
    return (min <= x) and (x < max);
  };
  auto isValidParticle = [&](const ActsFatras::Particle& p) {
    const auto eta = Acts::VectorHelpers::eta(p.unitDirection());
    const auto phi = Acts::VectorHelpers::phi(p.unitDirection());
    const auto rho = Acts::VectorHelpers::perp(p.position());
    // defined charge selection
    const bool validNeutral = (p.charge() == 0) and not m_cfg.removeNeutral;
    const bool validCharged = (p.charge() != 0) and not m_cfg.removeCharged;
    const bool validCharge = validNeutral or validCharged;
    return validCharge and
           within(p.transverseMomentum(), m_cfg.ptMin, m_cfg.ptMax) and
           within(std::abs(eta), m_cfg.absEtaMin, m_cfg.absEtaMax) and
           within(eta, m_cfg.etaMin, m_cfg.etaMax) and
           within(phi, m_cfg.phiMin, m_cfg.phiMax) and
           within(std::abs(p.position().z()), m_cfg.absZMin, m_cfg.absZMax) and
           within(rho, m_cfg.rhoMin, m_cfg.rhoMax);
  };

  std::size_t allParticles = 0;
  std::size_t selectedParticles = 0;

  selected.reserve(input.size());
  for (const auto& inputVertex : input) {
    allParticles += inputVertex.incoming.size();
    allParticles += inputVertex.outgoing.size();

    SimVertex vertex(inputVertex.position4, inputVertex.process);
    // copy selected particles over
    std::copy_if(inputVertex.incoming.begin(), inputVertex.incoming.end(),
                 std::back_inserter(vertex.incoming), isValidParticle);
    std::copy_if(inputVertex.outgoing.begin(), inputVertex.outgoing.end(),
                 std::back_inserter(vertex.outgoing), isValidParticle);

    // only retain vertex if it still contains particles
    if (vertex.incoming.empty() and vertex.outgoing.empty()) {
      continue;
    }

    selectedParticles += vertex.incoming.size();
    selectedParticles += vertex.outgoing.size();
    selected.push_back(std::move(vertex));
  }

  ACTS_DEBUG("event " << ctx.eventNumber << " selected " << selectedParticles
                      << " from " << allParticles << " particles");

  ctx.eventStore.add(m_cfg.outputEvent, std::move(selected));
  return ProcessCode::SUCCESS;
}
