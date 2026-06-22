# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository nature

AAS is a **schema-only repository** — no source code, no build, no application. It defines the **Analog Agnostic Structure**: a JSON Schema 2020-12 specification for *generic analog / linear ICs* — operational amplifiers, comparators, instrumentation & difference amplifiers, programmable-gain amplifiers, buffers / line drivers, sample-and-hold amplifiers, analog switches & multiplexers, and data converters (ADC / DAC). Work here is editing JSON Schema files and Markdown docs.

AAS is the home for analog ICs that are **not** power-conversion control parts (those live in **CTAS**) and **not** discrete power semiconductors (those live in **SAS**). The boundary test: an op amp / comparator / ADC is a domain-agnostic analog building block; a PWM/LLC/PFC controller, gate driver, current-sense amp, voltage reference or shunt regulator is a power-control IC and belongs in CTAS.

The only executable is `scripts/validate.py` (the validation gate). Run it after every schema change:

```bash
pip install jsonschema referencing
python3 scripts/validate.py
```

## Layout — discriminate the family at the TOP level (the SAS pattern)

AAS follows **SAS**, not CTAS: the device family is discriminated by **which top-level field is present** (`operationalAmplifier` | `comparator` | … | `adc` | `dac`), not by an internal `function.category`. This is correct because the families have genuinely disjoint parameter sets (an ADC has resolution/SNR/sample-rate; a switch has Ron/charge-injection — they share almost nothing), so a per-family `oneOf` of closed objects beats one shared payload with category-gated sub-objects.

- `schemas/AAS.json` — top wrapper `{ inputs, <family>, outputs }` with `anyOf` (bare component | full doc) + a `oneOf` over the 11 family discriminators. Every valid AAS document is also a valid PEAS document (the `analog` branch).
- `schemas/<family>.json` — one file per family (`operationalAmplifier`, `comparator`, `instrumentationAmplifier`, `differenceAmplifier`, `programmableGainAmplifier`, `buffer`, `sampleHold`, `analogSwitch`, `multiplexer`, `adc`, `dac`). Each holds `{ manufacturerInfo, distributorsInfo }`; `manufacturerInfo.datasheetInfo = { part, electrical, thermal, mechanical }`; `electrical` is the family-specific parametric block.
- `schemas/utils.json` — **AAS-owned** shared sub-schemas: identification (`part`, `thermal`, `mechanical`, mirroring SAS over the PEAS `datasheetInfo*` bases) and the electrical **cores** (`supply`, `amplifierCommon`, `switchCore`, `converterDynamics`, `gainPoint`) plus the family enums (`logicFamily`, `digitalInterface`, `referenceSource`, `inputType`).
- `schemas/inputs.json` + `schemas/inputs/designRequirements.json` — the design-requirements seed (built on the PEAS `designRequirementsBase` mixin) with a `deviceType` discriminator + generic selection seeds.
- `schemas/outputs.json` — small selection-oriented output surface (`powerDissipation`, `thermal`, `selection`), each wrapping the PEAS `outputBase` provenance shell.
- `docs/schema.md` — field-by-field reference. **Keep in sync with the schema files.**
- `examples/*.json` — worked whole-document examples. **Keep in sync** and re-run `validate.py`.

There is **no** `data/` directory: AAS is a building-block / type repo. Finished orderable analog-IC parts live in **`TAS/data/`** (e.g. `analog_ics.ndjson`), validated against the per-family schemas.

## Sibling-repo layout and PEAS relationship

AAS is a sibling of MAS / CAS / SAS / RAS / CTAS / CONAS under the PEAS umbrella at `/home/alf/PSMA/`. It is **not** self-contained the way MAS is:

- **Reuse PEAS shared primitives by absolute `$id` URI**, never by relative path: `https://psma.com/peas/utils.json#/$defs/{datasheetInfoPartBase, datasheetInfoThermal, datasheetInfoMechanical, dimensionWithTolerance, manufacturerInfo (field pointers), distributorInfo, designRequirementsBase}` and `https://psma.com/peas/outputs/outputBase.json`.
- **Own the analog-family enums here** (the CONAS/CTAS pattern). If a vocabulary is used only by analog ICs, add it to `AAS/schemas/utils.json`, not PEAS.
- PEAS pins the `analog` branch to `https://psma.com/aas/AAS.json` and the seed to `https://psma.com/aas/inputs/designRequirements.json`.

## Schema editing rules (the strictness philosophy)

- **Closed objects everywhere.** Every object is `additionalProperties: false` (or `unevaluatedProperties: false` on the extension branch when it extends a shared base via `allOf`). Never leave an object open.
- **Family `oneOf`, not a flat bag.** Each family is its own closed file under the top-level `oneOf`. Electrical parameters that genuinely recur across families live in a `utils.json` core (`amplifierCommon`, `switchCore`, `converterDynamics`) that the family extends by `allOf`; family-unique parameters live in the family file.
- **No redundancy / no double-modelling.** Before adding a field, check it is not derivable from an existing one. Established eliminations: **SINAD = power-sum(SNR, THD)** and **ENOB = (SINAD−1.76)/6.02** are derived — store only SNR/THD/SFDR (`converterDynamics`). **Total supply current = quiescentCurrentPerChannel × numberOfChannels** — store per-channel only. **GBW ≡ unity-gain bandwidth** for unity-gain-stable VFAs — store once. Rail-to-rail in/out are kept as booleans (the parametric-search filters) instead of also modelling the V_CM range / output swing.
- **Gain-dependent specs are `(gain, value)` arrays** (`gainPoint`), not scalars, for in-amps and PGAs whose bandwidth/CMRR vary strongly with gain.
- **Condition-qualified scalars carry their condition** — comparator `propagationDelay` carries overdrive + load capacitance; converter `dynamics` carries `measurementFrequency`; noise carries `voltageNoiseDensityFrequency`.
- **Enums are complete unions of fixed strings.** When a real part needs a value the enum lacks, add it (don't shoehorn). Verify against the datasheet.
- **When you add or rename a field, update `docs/schema.md` and any `examples/*.json` in the same change, then re-run `validate.py`.**

## Provenance of the field set

The parameter set was derived from a datasheet survey across every family: op amps (TI OPA192/OPA387/OPA333/TLV9061, ADI ADA4522/AD8605, Microchip MCP6V01); comparators (LM393, TLV3201, LT1716); in-amps (AD620/AD8421, INA128/826); difference amps (INA117/149, AD8276/8278/8475); PGAs (PGA280, MCP6S2x, AD8253/8231, LTC6915); buffers/line drivers (BUF634, LMH6321, ADA4870); sample-and-hold (LF398, AD783/585, AD9101); analog switches/muxes (ADG1419, TS5A23159, ADG706, 74HC4051); ADCs (ADS1115, AD7124, ADS8688, AD9226); DACs (AD5681, DAC8568, MCP4725, AD9744, LTC2668).
