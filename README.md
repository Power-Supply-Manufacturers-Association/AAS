# AAS — Analog Agnostic Structure

> A vendor- and simulator-neutral description of a **generic analog / linear IC** — the op amps, comparators, data converters and analog switches that sit between the power stage and the controller.

AAS is a [JSON Schema 2020-12](https://json-schema.org/draft/2020-12/schema) data model for one **analog IC**. It is a sibling of MAS (magnetics), CAS (capacitors), SAS (semiconductors), RAS (resistors), CTAS (control ICs) and CONAS (connectors) under the **PEAS** umbrella. Every valid AAS document is also a valid [PEAS](https://github.com/Power-Supply-Manufacturers-Association/PEAS) document — the `analog` branch.

## What belongs here

AAS is the home for **domain-agnostic analog building blocks**:

| Family (top-level discriminator) | Examples |
| --- | --- |
| `operationalAmplifier` | OPA192, TLV9061, ADA4522 |
| `comparator` | LM393, TLV3201, LT1716 |
| `instrumentationAmplifier` | AD620, INA128, AD8421 |
| `differenceAmplifier` | INA117, AD8276, AD8475 |
| `programmableGainAmplifier` | PGA280, MCP6S26, AD8253 |
| `buffer` | BUF634, LMH6321 (line driver), AD8075 (video) |
| `sampleHold` | LF398, AD783, AD9101 |
| `analogSwitch` | ADG1419, TS5A23159, DG419 |
| `multiplexer` | ADG706, CD74HC4051, MAX4051 |
| `adc` | ADS1115, AD7124, ADS8688, AD9226 |
| `dac` | AD5681, DAC8568, MCP4725, AD9744 |

## What does NOT belong here

- **Power-control ICs** (PWM / LLC / PFC / phase-shift / sync-rectifier controllers, gate drivers, current-sense & isolated amplifiers, voltage references, shunt regulators, hot-swap / eFuse) → **CTAS**.
- **Discrete power semiconductors** (MOSFET / diode / IGBT / BJT) → **SAS**.

The boundary test: if the part's reason-for-being is *controlling or sensing a power converter*, it is CTAS. A generic op amp / comparator / ADC that shows up equally in audio, instrumentation and sensor front-ends is AAS.

## Shape

```
AAS document
  inputs       designRequirements seed (+ optional operatingPoints)
  <family>     exactly one of the 14 families above (the field name is the discriminator)
  outputs[]    per-operating-point results (powerDissipation, thermal, selection)
```

The family is discriminated by **which top-level field is present** (the SAS pattern), because the families have disjoint parameter sets. Identification (`part` / `thermal` / `mechanical`) and the recurring electrical cores (`supply`, `amplifierCommon`, `switchCore`, `converterDynamics`) live in `schemas/utils.json`; family-specific electricals live in the per-family files.

## Validate

```bash
pip install jsonschema referencing
python3 scripts/validate.py
```

Requires the sibling repos (at least PEAS) checked out alongside AAS in the `PSMA/` layout, because cross-repo `$ref`s resolve by absolute `https://psma.com/...` `$id`.

See `docs/schema.md` for the field-by-field reference and `examples/` for worked documents.
