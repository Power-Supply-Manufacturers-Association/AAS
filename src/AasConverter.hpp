#pragma once

// AasConverter — "generate a CIAS element (leaf) from an AAS analog component".
//
// aas_to_cias(peas, fidelity) returns a small CIAS brick (a "leaf") realising ONE analog block as
// ideal-atom PEAS components wired together — mirroring ras_to_cias / cas_to_cias / sas_to_cias for the
// AAS (analog) family. Supported so far: the comparator, as an ideal behavioural model carried in
// `comparator.behavioral` (outputHigh / outputLow / threshold / hysteresis). The leaf's single atom
// keeps the `analog.comparator.behavioral` block verbatim so the CIAS->simulator converter realises it
// (ngspice: a controlled switch with native Vt/Vh hysteresis).
//
// Input `peas` is a PEAS/AAS analog document — either {"analog": {"comparator": {...}}, ...} or the bare
// {"comparator": {...}}. Output is a CIAS leaf as JSON ({name, ports, components, connections}); it is
// consumed by the CIAS converter, never persisted, so it need not validate against CIAS.json.
//
// Convention: a comparator's pins are "inPlus", "inMinus", "out".

#include <nlohmann/json.hpp>
#include <string>
#include "Fidelity.hpp"

namespace AAS {

// peas: a PEAS/AAS analog document. Returns a CIAS leaf brick as JSON.
nlohmann::json aas_to_cias(const nlohmann::json& peas,
                           const PEAS::Fidelity& fidelity,
                           const std::string& name = "analog");

} // namespace AAS
