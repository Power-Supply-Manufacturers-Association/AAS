# AAS schema reference

Field-by-field reference for the Analog Agnostic Structure. Keep this in sync with the schema files and `examples/`.

All quantities are **SI base units** unless a field says otherwise: voltages in V, currents in A, capacitance in F, charge in C, resistance in Ohm, time in s, frequency/rate in Hz (or samples-per-second for converters), gains in dB or V/V as noted, temperatures in degC.

## Document shape (`AAS.json`)

A document is `{ inputs, <family>, outputs }`:

- `inputs` — `inputs.json`: a required `designRequirements` seed plus optional `operatingPoints[]`.
- `<family>` — **exactly one** of `operationalAmplifier`, `comparator`, `instrumentationAmplifier`, `differenceAmplifier`, `programmableGainAmplifier`, `buffer`, `sampleHold`, `analogSwitch`, `multiplexer`, `adc`, `dac`, `digitalPotentiometer`, `digitalIsolator`, `currentSensor`, `multiplier`, `summer`, `integrator` (17 families). The field name is the discriminator.
- `outputs[]` — array of `outputs.json`; `outputs[i]` aligns positionally with `inputs.operatingPoints[i]`.

`anyOf` allows either a **full document** (`inputs` + family + `outputs`) or a **bare component** (the family field alone, `maxProperties: 1`) — the latter is how a part record is stored in `TAS/data`.

Every family file has the same outer shape (mirroring SAS `mosfet.json`):

```
<family>.manufacturerInfo            name (req) + reference/status/description/orderCode/datasheetUrl/family/series/spiceModel + datasheetInfo (req)
<family>.manufacturerInfo.datasheetInfo   part (req) + electrical (req) + thermal + mechanical + pinout
<family>.distributorsInfo[]          PEAS distributorInfo
<family>.substitutesInfo[]           PEAS substituteInfo — successors + second sources
```

`substitutesInfo[]` (PEAS-RFC 0002) is the PEAS type every part family shares: `{partNumber (req), manufacturer, type, notes, source}`. `type: "successor"` means THIS part is superseded by the named one — one hop, as the manufacturer states it, never inferred from `status: obsolete`. The named part need not be in any catalogue (resolution is the referential pass's job), and the evidence is an ordinary `datasheetInfo.provenance[]` entry with `fields: ["substitutesInfo"]`. It rides along with a sourced part: a family object carrying only `substitutesInfo` is rejected by the seed `anyOf`.

`manufacturerInfo` field definitions are `$ref`-ed from `https://psma.com/peas/utils.json#/$defs/manufacturerInfo`. An empty family object `{}` is a valid pre-librarian seed.

## Shared identification (`utils.json`)

- **`part`** — extends PEAS `datasheetInfoPartBase` (`partNumber`, `series`, `case`, `description`) with `package` (manufacturer package designation) and `qualification` (grade, e.g. AEC-Q100).
- **`thermal`** — extends PEAS `datasheetInfoThermal` (`operatingTemperature` as a `dimensionWithTolerance` min/max in degC) with `thermalResistanceJunctionAmbient` (K/W) and `maximumPowerDissipation` (W).
- **`mechanical`** — extends PEAS `datasheetInfoMechanical` (length/width/height/diameter/weight/shapeType/assemblyType) with `case`.
- **`pinout`** — optional per-pin function map, one entry per package pin/ball, built on the **shared PEAS `pin` base** (`https://psma.com/peas/utils.json#/$defs/pin`, hoisted from AAS 2026-08): `pin` (req, as printed: `"1"`, `"A2"`), `function` (req, the family-wide PEAS `pinFunction` vocabulary — all previous AAS values plus power-semi/connector/magnetics roles), `name` (datasheet label verbatim), `polarity` (activeHigh/activeLow, control pins); AAS extends each entry with `outputStage` (pushPull/openDrain/openCollector — decides drop-in compatibility for outputs). This is the deciding axis for IC swaps: same package ≠ same pinout (three mutually incompatible SOIC-8 single-op-amp pinouts exist). Register-map compatibility is deliberately out of scope.

## Shared electrical cores (`utils.json`)

- **`supply`** — `minimumSupplyVoltage` / `maximumSupplyVoltage` (total span (V+)−(V−)), `dualSupply` (bool: bipolar-capable), `quiescentCurrentPerChannel`, `shutdownSupplyCurrent`. Device total quiescent current = `quiescentCurrentPerChannel × numberOfChannels` (derived, not stored).
- **`amplifierCommon`** (base for the linear-amplifier families) — `numberOfChannels`, `inputOffsetVoltage`, `inputOffsetVoltageDrift` (V/K), `inputBiasCurrent`, `inputOffsetCurrent`, `commonModeRejectionRatio` (dB), `powerSupplyRejectionRatio` (dB), `slewRate` (V/s), `voltageNoiseDensity` (V/√Hz) + `voltageNoiseDensityFrequency`, `supply`.
- **`switchCore`** (base for `analogSwitch` + `multiplexer`) — `onResistance`, `onResistanceFlatness`, `onResistanceMatch` (three distinct specs), `offLeakageCurrent`, `chargeInjection` (C), `turnOnTime`, `turnOffTime`, `offIsolation` (dB), `crosstalk` (dB), `bandwidth`, `signalRange` (`dimensionWithTolerance`), `controlLogic` (`logicFamily`), `supply`.
- **`converterDynamics`** (used by `adc` + `dac`) — `signalToNoiseRatio`, `totalHarmonicDistortion`, `spuriousFreeDynamicRange` (all dB) + `measurementFrequency`. **SINAD and ENOB are derived, never stored.**
- **`gainPoint`** — `{ gain, value }`, one sample of a gain-dependent spec.
- Enums: **`logicFamily`** (CMOS / TTL / TTL/CMOS), **`digitalInterface`** (SPI / I2C / parallel / microwire / pinStrap / serialLVDS / parallelLVDS / JESD204 / upDown / pushButton — / bitstreamCmos / bitstreamLvds / manchester — `upDown` = the chip-select + up/down + increment interface of MCP40xx / AD5220-style digital potentiometers, `pushButton` = debounced manual up/down inputs such as AD5228 / DS1809, `bitstreamCmos` / `bitstreamLvds` = the 1-bit output (with its clock) of a delta-sigma modulator at CMOS or LVDS levels (AMC1306M05 / AMC1306E05, AD7403), `manchester` = a Manchester-coded modulator bitstream that carries its own clock (AMC1306E25 / AMC1336 class)), **`referenceSource`** (internal / external / supply), **`inputType`** (singleEnded / differential / pseudoDifferential).

## Per-family `electrical`

**operationalAmplifier** = `amplifierCommon` + `gainBandwidthProduct` (≡ unity-gain BW for VFA), `openLoopGain` (dB), `outputCurrent`, `architecture` (generalPurpose/precision/lowNoise/lowPower/highSpeed/zeroDrift/currentFeedback/fullyDifferential/transimpedance), `inputStage` (CMOS/JFET/bipolar/BiCMOS), `railToRailInput`, `railToRailOutput`, `unityGainStable` (+ `minimumStableGain`), `hasShutdown`.

**comparator** (standalone) = `numberOfChannels`, `propagationDelay` (optional since 2026-07 — was the only required performance leaf in any family and blocked imports; carry the conditions when present) + `propagationDelayOverdrive` + `propagationDelayLoadCapacitance`, `inputOffsetVoltage`, `inputBiasCurrent`, `hysteresis` (absent = none), `outputStage` (pushPull/openDrain/openCollector/complementary), `commonModeVoltageRange`, `type` (generalPurpose/lowPower/highSpeed/precision/window), `hasLatch`, `supply`.

## Ideal behavioural model (`comparator.behavioral`)

Separate from the datasheet `electrical` selection parameters, a comparator may carry an optional, simulator-agnostic **`behavioral`** block describing an *ideal* control block for circuit simulation — a thresholded, hysteretic switch `out = (V(inPlus) - V(inMinus) > threshold ± hysteresis) ? outputHigh : outputLow`. Fields: `outputHigh` (req), `outputLow` (req), `threshold` (default 0), `hysteresis` (half-width, default 0). A document may carry `behavioral` **without** `manufacturerInfo` (an ideal block, not a catalog part). A simulator backend realises it natively (ngspice: a controlled switch with `Vt`/`Vh`); see `examples/ideal-comparator-behavioral.json`. The mirror on the controller side is `CTAS controller.behavioral`.

## Behavioural control-computing blocks (`multiplier`, `integrator`, `summer`)

Three additional families are **ideal behavioural blocks** for circuit simulation (no parametric part catalog — just a `behavioral` section, or an empty seed): the building blocks of a control law expressed in the netlist.

- **multiplier** (pins `inA`/`inB`/`out`) — `out = gain·V(inA)·V(inB)`.
- **integrator** (pins `in`/`out`) — `out = clamp(initial + gain·∫(V(in) − reference)dt, outputLow, outputHigh)` (a loop compensator).
- **summer** (pins `inA`/`inB`/`out`) — `out = gainA·V(inA) + gainB·V(inB)` (a difference when `gainB` is negative).

With these + `comparator` you can build current loops, voltage loops, and polarity-aware control entirely in the (simulator-agnostic) netlist; a backend maps each to its own multiply / integrate / sum / compare block. See `examples/ideal-{multiplier,integrator,summer}-behavioral.json`.

**instrumentationAmplifier** = `amplifierCommon` + `gainSetMethod` (optional — enforcement blocked: the 49 catalogued in-amps lack it, librarian backlog; values: singleResistor/fixed/pinProgrammable/digitallyProgrammable), `gainEquationConstant` (k in G=1+k/R_G), `minimumGain`/`maximumGain`, `bandwidthVsGain[]` & `commonModeRejectionRatioVsGain[]` (`gainPoint` arrays), `outputOffsetVoltage`, `inputCommonModeVoltageRange`, `architecture` (threeOpAmp/twoOpAmp/currentFeedback/zeroDrift).

**differenceAmplifier** = `amplifierCommon` + `gain` (req, fixed), `gainConfigurability` (fixed/pinSelectable), `gainOptions[]`, `gainError` (fraction), `gainErrorDrift` (1/K), `bandwidth`, `inputCommonModeVoltageRange` (may exceed rails), `inputOverloadVoltage`, `outputType` (singleEnded/differential). **Isolated amplifiers** (AMC1301 / AMC1311 / AMC3301 class) are difference amplifiers that also carry `isolation` (the shared PEAS `isolationRating`, see below), `linearInputRange` (`dimensionWithTolerance`, V, signed limits: `{-0.25, 0.25}` for ±250 mV), `nonlinearity` (fraction, maximum magnitude) and `outputSideSupply` (AAS `supply` of the output side VDD2; `supply` then holds the input side VDD1). See `examples/amc1301-isolated-amplifier.json`.

**programmableGainAmplifier** = `amplifierCommon` + `gainValues[]` (optional — enforcement blocked: the 44 catalogued PGAs lack it, librarian backlog), `gainProgression` (binary/decade/scope/arbitrary), `digitalInterface[]`, `inputMultiplexerChannels`, `bandwidthVsGain[]`, `commonModeRejectionRatioVsGain[]`, `architecture` (opAmpPga/instrumentationPga).

**buffer** = `amplifierCommon` + `bufferType` (openLoopBuffer/closedLoopBuffer/lineDriver/videoBuffer/differentialDriver/powerBuffer), `bandwidth`, `gainConfiguration` (fixedUnity/fixedGain/adjustableGain) + `fixedGainValue`, `continuousOutputCurrent`, `peakOutputCurrent`, `outputImpedance`, `adjustableCurrentLimit`, `outputType`, `thermalShutdown`.

**sampleHold** = `amplifierCommon` + `subType` (sampleAndHold/trackAndHold), `acquisitionTime` + `acquisitionAccuracy`, `droopRate` (V/s), `holdCapacitor` (internal/external) + `holdCapacitance`, `apertureDelay`, `apertureJitter`, `holdStep`, `feedthrough` (dB), `bandwidth`, `gain`, `switchTechnology` (CMOS/FET/diodeBridge/bipolar).

Like the comparator, `sampleHold` also carries an optional ideal **`behavioral`** block (valid without `manufacturerInfo`, per PEAS-RFC 0001): `mode` (req: `trackWhileActive` = transparent while the trigger is active, holds otherwise / `sampleOnEdge` = captures on the active edge only), `threshold` (req, V on the trigger input), `polarity` (req: activeHigh/activeLow). A backend realises it as switch + hold capacitor + unity buffer (LTspice) or the XSPICE sample idiom (ngspice). See `examples/ideal-samplehold-behavioral.json`.

**analogSwitch** = `switchCore` + `switchConfiguration` (req: SPST-NO/SPST-NC/SPDT/DPST/DPDT/SP3T/SP4T/DP3T/DP4T/4PST), `numberOfSwitches`, `totalHarmonicDistortion` (dB).

**multiplexer** = `switchCore` + `multiplexerConfiguration` (req, e.g. "8:1"), `numberOfChannels`, `signalConfiguration` (`inputType`), `breakBeforeMakeTime`, `transitionTime`.

**adc** = `resolution` (req, bits) + `architecture` (req: SAR/deltaSigma/pipeline/flash/dualSlope/folding), `sampleRate`, `numberOfChannels`, `inputType`, `integralNonlinearity`/`differentialNonlinearity` (LSB), `offsetError`/`gainError`, `dynamics` (`converterDynamics`, high-speed parts), `effectiveResolution` (precision ΔΣ parts), `fullScaleRange`, `hasProgrammableGainAmplifier`, `referenceSource` + `referenceVoltage`, `digitalInterface[]`, `supply` (analog rail) + `digitalSupplyVoltage`, `powerConsumption`, `isolation` (PEAS `isolationRating`, isolated converters and isolated delta-sigma modulators only: AMC13xx, AD740x, ADuM770x; absent on a non-isolated ADC). A modulator's bitstream output is `digitalInterface` `bitstreamCmos` / `bitstreamLvds` / `manchester`.

**dac** = `resolution` (req) + `architecture` (req: stringDac/r2r/multiplying/sigmaDelta/segmented/currentSteering), `numberOfChannels`, `outputType` (voltageBuffered/voltageUnbuffered/current), `settlingTime`, `updateRate`, `integralNonlinearity`/`differentialNonlinearity`, `guaranteedMonotonic`, `gainError`, `dynamics`, `referenceSource` + `referenceVoltage`, `digitalInterface[]`, `supply` + `digitalSupplyVoltage`, `powerConsumption`, `hasEeprom`.

**digitalPotentiometer** (DigiPOT / digital rheostat: a CMOS resistor ladder whose tap is set over a digital interface; an IC, not a RAS resistive track) = `numberOfChannels` (req), `configuration` (potentiometer: A/B/W per channel | rheostat: two terminals), `numberOfPositions` (req, taps as the datasheet states them: 256, 129, 33; bits are derived, never stored), `totalResistance` (req, `dimensionWithTolerance`, Ohm: one resistance option per orderable code), `resistanceTolerance` (fraction), `temperatureCoefficient` (rheostat mode) and `ratiometricTemperatureCoefficient` (divider mode), both in **ppm/K** like RAS (a documented exception to SI), `wiperResistance` (`dimensionWithTolerance`, Ohm), `terminalVoltageRange` (`dimensionWithTolerance`, **absolute** V referred to ground; written only when the datasheet prints numbers — a rail-relative GND…VDD range is omitted, not filled with supply values), `maximumTerminalCurrent` / `maximumPulsedTerminalCurrent` (A), `rheostat{Integral,Differential}Nonlinearity` and `potentiometer{Integral,Differential}Nonlinearity` (`dimensionWithTolerance` in LSB: `nominal` = typical magnitude, `minimum`/`maximum` = the guaranteed limits with their signs, e.g. R-INL `{-2, 0.25, +2}`), `taper` (linear/logarithmic; absent = not sourced) + `logarithmicStepSize` (dB), `bandwidth` (Hz, midscale), `totalHarmonicDistortion` (dB), `memory` (closed `oneOf` on a pinned `type`: `volatile` {`powerOnPosition` 0..1} | `nonVolatile` {`technology` (req: eeprom/flash/oneTimeProgrammable/multipleTimeProgrammable), `writeEndurance`, `dataRetention` (s)}), `digitalInterface[]`, `supply` (AAS supply), `supplyCurrent` (A, the **device** I_DD, typical column), `digitalSupplyVoltage` (`dimensionWithTolerance`, V, VLOGIC parts). `supplyCurrent` and `supply.quiescentCurrentPerChannel` are **mutually exclusive** (the same current, never modelled twice; digipot datasheets print a device figure). Pins use the PEAS `pinFunction` roles `terminalA` / `terminalB` / `wiper`. The resistive field names repeat RAS `potentiometer` names and units but are defined locally (AAS `$ref`s PEAS only). See `examples/ad5160-digital-potentiometer.json`.

**digitalIsolator** (logic channels across a galvanic-isolation barrier, optionally with integrated isolated power — ADI isoPower, TI ISOW — or a bus transceiver; optocouplers, isolated gate drivers (CTAS `gateDriver`) and isolated amplifiers / modulators are NOT this family) = `forwardChannels` (req) + `reverseChannels` (req), counted from side 1 (forward = input on side 1, output on side 2), `bidirectionalChannels` (open-drain I2C-style); at least one channel must be non-zero and the total is derived, never stored. `interface` (generalPurpose / i2c / smbus / pmbus / spi / usb / lvds / can / rs485 / rs422 / rs232 / lin), `barrierTechnology` (capacitive / inductive / giantMagnetoresistive), `maximumDataRate` (bit/s), `propagationDelay` (`dimensionWithTolerance`, s), `pulseWidthDistortion`, `channelToChannelSkew`, `partToPartSkew` (s, maximum), `defaultOutputState` (high/low: the output when the input side is unpowered; TI 'F' suffix = low), `outputEnable`, `side1Supply` / `side2Supply` (`isolatorSideSupply`: `minimumSupplyVoltage`, `maximumSupplyVoltage`, `supplyCurrent[]` rows `{dataRate (req), supplyVoltage, current (req, dimensionWithTolerance)}` — an isolator's supply current depends on data rate and voltage, so it is a table, never one number), `integratedIsolatedPower` (`outputVoltage` req, `maximumOutputCurrent`), `isolation` (PEAS `isolationRating`). Timing values are the 5 V figures where several supplies are tabulated (condition in provenance). See `examples/iso7741-digital-isolator.json`.

**currentSensor** (active magnetic current sensor / transducer: Hall, fluxgate or magnetoresistive, open or closed loop, integrated-conductor IC such as Allegro ACS7xx or aperture module such as LEM LA. Passive CTs and Rogowski coils are MAS magnetics; non-isolated shunt amplifiers are CTAS `currentSenseAmplifier`; magnetic FIELD sensors, output per tesla, are out of scope) = `measuringRange` (req, `dimensionWithTolerance` in A as signed limits `{minimum -10, maximum 10}`; bidirectional vs unidirectional is derived, not stored) + `output` (req, closed `oneOf` on a pinned `type`: `analogVoltage` {`sensitivity` (req, V/A), `sensitivityError[]`, `zeroCurrentOutputVoltage` (V) or `zeroCurrentOutputRatio` (fraction of supply), `ratiometric`, `offsetVoltageError[]` {`maximum`/`typical` V + conditions}, `outputConfiguration` singleEnded/differential} | `analogCurrent` {`conversionRatio` (req, I_S/I_P, 0.001 for 1:1000), `secondaryNominalCurrent`, `offsetCurrent`, `secondaryCoilResistance[]` {`resistance`, `ambientTemperature`}, `measuringResistance[]` {`supplyRailVoltage`, `primaryCurrent`, `ambientTemperature`, `minimum`, `maximum`} — a footnote-limited cell records the limited current} | `digital` {`digitalInterface[]` (req), `resolution`, `lsbCurrent`}), plus `numberOfChannels`, `sensingElement` (hall / fluxgate / anisotropicMagnetoresistive / giantMagnetoresistive / tunnelMagnetoresistive), `loopType` (openLoop/closedLoop), `primaryConductor` (integrated / aperture / externalConductor), `primaryAperture` (closed `oneOf` on pinned `shape`: `round` {`diameter`} | `rectangular` {`width`, `height`}; allowed only with `primaryConductor: aperture`), `primaryConductorResistance` (Ohm), `primaryConductorInductance` (H), `primaryNominalCurrent` (A RMS), `totalError[]` (`errorRow`: `maximum` and/or `typical` fraction, `reference` (req: ofReading / ofNominalCurrent / ofFullScale), `supplyRailVoltage`, `ambientTemperature` — errors are stored per printed condition because sensor datasheets tabulate them per temperature band or supply), `linearityError` (fraction), `bandwidth` (Hz) + `bandwidthCriterion` (dB, negative: −3 Allegro, −1 LEM), `propagationDelay`, `responseTime` (to 90 %), `riseTime` (s), `noiseDensity` (A/√Hz), `overcurrentFaultOutput`, `supply` (AAS supply; a ±12…15 V transducer is the 22.8…31.5 V span with `dualSupply`), `supplyCurrent` (device figure, `dimensionWithTolerance`; closed-loop: excluding the secondary current; mutually exclusive with `supply.quiescentCurrentPerChannel`), `isolation` (PEAS `isolationRating`). IP+ / IP− pins use PEAS `pinFunction` `currentSensePositive` / `currentSenseNegative`. See `examples/acs724-10ab-current-sensor.json` and `examples/la55p-current-transducer.json`.

### Isolation barrier (`electrical.isolation`)

`digitalIsolator`, `currentSensor`, `adc` and `differenceAmplifier` carry the barrier rating at `electrical.isolation`, the same path CTAS isolated controllers use, so one checker reads every family. The block is **PEAS `isolationRating`** (`https://psma.com/peas/utils.json#/$defs/isolationRating`, hoisted from CTAS in 2026-10 with the CTAS key names kept), closed, every key optional: `isolationType` (PEAS `isolationClass`: functional / basic / supplementary / double / reinforced; a non-isolated part omits the block), `withstandVoltageRms` (V_ISO) + `withstandVoltageDuration` (s), `workingVoltage` (V_IORM, peak or DC), `workingVoltageRms` (V_IOWM), `transientVoltage` (V_IOTM), `impulseVoltage` (V_IMP / U_Ni), `surgeVoltage` (V_IOSM / V_SURGE), `cmti` (V/s), `barrierCapacitance` (F), `barrierResistance` (Ohm), `creepage`, `clearance`, `distanceThroughInsulation` (m), `comparativeTrackingIndex` (V), `materialGroup` (I / II / IIIa / IIIb), `pollutionDegree`, `ratedInsulationVoltage[]` (`{voltage (req, V RMS), insulationClass, overvoltageCategory, pollutionDegree, standard}`, one per insulation-coordination row). Peak and RMS are fixed by each key's name and never mixed.

## Inputs / outputs

`inputs.designRequirements` = PEAS `designRequirementsBase` + `deviceType` (req, one of the 17 families) + generic seeds: `minimumChannels`, `supplyVoltage` (`dimensionWithTolerance`), `maximumQuiescentCurrent`, `maximumInputOffsetVoltage`, `minimumBandwidth`, `minimumResolution`.

`outputs` (each block wraps PEAS `outputBase` `{origin, methodUsed}`): `powerDissipation` (`quiescentLoss`/`outputLoss`/`totalLoss`), `thermal` (`junctionTemperature`, `passes`), `selection` (`supplyVoltageMargin`, `channelsSatisfied`, `passes`).
## Provenance (data-source trail)

Every `datasheetInfo` carries an optional `provenance` array recording where its data
came from. Optional and closed, so records without it remain valid. Each entry:

| field | meaning |
|---|---|
| `source` | `manufacturerDatasheet` · `manufacturerParametric` · `manufacturerDatabase` · `distributor` · `librarianEnrichment` · `scrape` · `manual` |
| `sourceName` | human-readable source, e.g. `"TI parametric API"`, `"WE - Passive Components.mdb"`, `"DigiKey"` |
| `sourceUrl` | URL the value came from (optional) |
| `retrievedDate` | `YYYY-MM-DD` (optional) |
| `fields` | which `datasheetInfo` fields this source supplied — for mixed-source records (optional) |

It is a **list**: a record may combine sources (e.g. specs from the datasheet, a rated
voltage from a distributor, a missing field back-filled by librarian enrichment). The
canonical definition lives in `PEAS/schemas/utils.json#/$defs/provenance` (mirrored in
`MAS/schemas/utils.json`, which is self-contained).
