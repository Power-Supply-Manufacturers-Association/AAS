#include "AasConverter.hpp"
#include <stdexcept>
#include <vector>

namespace AAS {
using nlohmann::json;

namespace {
// The analog payload, whether the document nests it under `analog` (PEAS) or carries it bare (AAS).
const json& analog_of(const json& peas) {
    if (peas.contains("analog") && peas.at("analog").is_object()) return peas.at("analog");
    return peas;
}
// A single-atom leaf: ONE analog atom (carrying `{<kind>:{behavioral}}` verbatim) with each named pin
// exposed as a like-named port. The CIAS converter realises the atom per backend.
json single_atom_leaf(const std::string& name, const std::string& kind, const json& block,
                      const std::vector<std::string>& pins) {
    json atom;
    json& b = atom["analog"][kind];
    b = json::object();
    if (block.contains("behavioral")) b["behavioral"] = block.at("behavioral");

    json leaf;
    leaf["name"] = name;
    leaf["ports"] = json::array();
    leaf["components"] = json::array({ json{{"name","C"}, {"data", atom}} });
    leaf["connections"] = json::array();
    for (const auto& p : pins) {
        leaf["ports"].push_back(json{{"name", p}});
        leaf["connections"].push_back(json{{"name", p}, {"endpoints", json::array({
            json{{"component","C"}, {"pin", p}}, json{{"port", p}} })}});
    }
    return leaf;
}
} // namespace

json aas_to_cias(const json& peas, const PEAS::Fidelity& /*fidelity*/, const std::string& name) {
    const json& a = analog_of(peas);
    // The portable analog control blocks: comparator (thresholded switch), multiplier (out=inA·inB),
    // integrator (out=∫(inPlus−inMinus)). Each lowers to a one-atom leaf carrying its `behavioral` block.
    if (a.contains("comparator"))
        return single_atom_leaf(name, "comparator", a.at("comparator"), {"inPlus", "inMinus", "out"});
    if (a.contains("multiplier"))
        return single_atom_leaf(name, "multiplier", a.at("multiplier"), {"inA", "inB", "out"});
    if (a.contains("summer"))
        return single_atom_leaf(name, "summer", a.at("summer"), {"inA", "inB", "out"});
    if (a.contains("integrator"))
        return single_atom_leaf(name, "integrator", a.at("integrator"), {"in", "out"});
    throw std::runtime_error("aas_to_cias: analog document has no supported block "
                             "(comparator / multiplier / integrator / summer)");
}

} // namespace AAS
