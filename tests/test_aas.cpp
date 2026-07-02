// Catch2 tests for aas_to_cias — the AAS analog-block -> CIAS leaf lowering.
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <nlohmann/json.hpp>
#include "AasConverter.hpp"

using json = nlohmann::json;
using Catch::Matchers::ContainsSubstring;

namespace {
json comparator_doc(bool with_behavioral) {
    json cmp = json::object();
    if (with_behavioral)
        cmp["behavioral"] = {{"outputHigh", 5.0}, {"outputLow", 0.0}, {"hysteresis", 0.01}};
    else
        cmp["manufacturerInfo"] = {{"name", "TI"}};
    return {{"analog", {{"comparator", cmp}}},
            {"inputs", {{"designRequirements", json::object()}}}};
}
} // namespace

TEST_CASE("comparator with behavioral lowers to a one-atom leaf", "[aas]") {
    const json leaf = AAS::aas_to_cias(comparator_doc(true), PEAS::Fidelity(PEAS::Fidelity::Origin::REQUIREMENTS), "U1");
    CHECK(leaf.at("name") == "U1");
    CHECK(leaf.at("ports").size() == 3);        // inPlus / inMinus / out
    CHECK(leaf.at("components").size() == 1);
    const json& atom = leaf.at("components")[0].at("data");
    CHECK(atom.at("analog").at("comparator").at("behavioral").at("outputHigh") == 5.0);
    // every pin is exposed as a like-named port net
    CHECK(leaf.at("connections").size() == 3);
}

TEST_CASE("datasheet-only comparator (no behavioral) throws — H11a regression", "[aas]") {
    CHECK_THROWS_WITH(AAS::aas_to_cias(comparator_doc(false), PEAS::Fidelity(PEAS::Fidelity::Origin::REQUIREMENTS), "U1"),
                      ContainsSubstring("no behavioral block"));
}

TEST_CASE("bare AAS payload (no PEAS 'analog' wrapper) also lowers", "[aas]") {
    json bare = {{"comparator", {{"behavioral", {{"outputHigh", 3.3}, {"outputLow", 0.0}}}}}};
    const json leaf = AAS::aas_to_cias(bare, PEAS::Fidelity(PEAS::Fidelity::Origin::REQUIREMENTS), "U2");
    CHECK(leaf.at("components")[0].at("data").at("analog").at("comparator")
              .at("behavioral").at("outputHigh") == 3.3);
}

TEST_CASE("unsupported analog block throws", "[aas]") {
    json doc = {{"analog", {{"operationalAmplifier", json::object()}}}};
    CHECK_THROWS_WITH(AAS::aas_to_cias(doc, PEAS::Fidelity(PEAS::Fidelity::Origin::REQUIREMENTS), "U3"),
                      ContainsSubstring("no supported block"));
}

TEST_CASE("integrator behavioral block is carried verbatim", "[aas]") {
    json doc = {{"analog", {{"integrator", {{"behavioral",
                   {{"gain", 1000.0}, {"outputLow", 0.0}, {"outputHigh", 2.5}}}}}}}};
    const json leaf = AAS::aas_to_cias(doc, PEAS::Fidelity(PEAS::Fidelity::Origin::REQUIREMENTS), "G1");
    CHECK(leaf.at("ports").size() == 2);        // in / out
    CHECK(leaf.at("components")[0].at("data").at("analog").at("integrator")
              .at("behavioral").at("gain") == 1000.0);
}
