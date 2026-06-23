#include "AasConverter.hpp"
#include <stdexcept>

namespace AAS {
using nlohmann::json;

namespace {
// The comparator payload, whether the document nests it under `analog` (PEAS) or carries it bare (AAS).
const json& comparator_of(const json& peas) {
    if (peas.contains("analog") && peas.at("analog").is_object()
        && peas.at("analog").contains("comparator"))
        return peas.at("analog").at("comparator");
    if (peas.contains("comparator")) return peas.at("comparator");
    throw std::runtime_error("aas_to_cias: analog document has no 'comparator' (only the comparator "
                             "family is supported so far)");
}
} // namespace

json aas_to_cias(const json& peas, const PEAS::Fidelity& /*fidelity*/, const std::string& name) {
    const json& cmp = comparator_of(peas);

    // One ideal comparator atom. Keep the agnostic `behavioral` block verbatim so the CIAS converter
    // (and every backend) reads the same rails / threshold / hysteresis. A part-only document (no
    // behavioral block) lowers to a default ideal comparator — the converter supplies 0..5 V defaults.
    json atom;
    json& bcmp = atom["analog"]["comparator"];
    bcmp = json::object();
    if (cmp.contains("behavioral")) bcmp["behavioral"] = cmp.at("behavioral");

    auto pinPort = [](const char* port, const char* pin) {
        return json{{"name", port}, {"endpoints", json::array({
            json{{"component", "C"}, {"pin", pin}}, json{{"port", port}} })}}; };

    json leaf;
    leaf["name"] = name;
    leaf["ports"] = json::array({ json{{"name","inPlus"}}, json{{"name","inMinus"}}, json{{"name","out"}} });
    leaf["components"] = json::array({ json{{"name","C"}, {"data", atom}} });
    leaf["connections"] = json::array({ pinPort("inPlus","inPlus"), pinPort("inMinus","inMinus"),
                                        pinPort("out","out") });
    return leaf;
}

} // namespace AAS
