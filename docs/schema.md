# AAS schema reference

Field-by-field reference for the Analog Agnostic Structure. Keep this in sync with the schema files and `examples/`.

All quantities are **SI base units** unless a field says otherwise: voltages in V, currents in A, capacitance in F, charge in C, resistance in Ohm, time in s, frequency/rate in Hz (or samples-per-second for converters), gains in dB or V/V as noted, temperatures in degC.

## Document shape (`AAS.json`)

A document is `{ inputs, <family>, outputs }`:

- `inputs` — `inputs.json`: a required `designRequirements` seed plus optional `operatingPoints[]`.
- `<family>` — **exactly one** of `operationalAmplifier`, `comparator`, `instrumentationAmplifier`, `differenceAmplifier`, `programmableGainAmplifier`, `buffer`, `sampleHold`, `analogSwitch`, `multiplexer`, `adc`, `dac`. The field name is the discriminator.
- `outputs[]` — array of `outputs.json`; `outputs[i]` aligns positionally with `inputs.operatingPoints[i]`.

`anyOf` allows either a **full document** (`inputs` + family + `outputs`) or a **bare component** (the family field alone, `maxProperties: 1`) — the latter is how a part record is stored in `TAS/data`.

Every family file has the same outer shape (mirroring SAS `mosfet.json`):

```
<family>.manufacturerInfo            name (req) + reference/status/description/orderCode/datasheetUrl/family/series/spiceModel + datasheetInfo (req)
<family>.manufacturerInfo.datasheetInfo   part (req) + electrical (req) + thermal + mechanical
<family>.distributorsInfo[]          PEAS distributorInfo
```

`manufacturerInfo` field definitions are `$ref`-ed from `https://psma.com/peas/utils.json#/$defs/manufacturerInfo`. An empty family object `{}` is a valid pre-librarian seed.

## Shared identification (`utils.json`)

- **`part`** — extends PEAS `datasheetInfoPartBase` (`partNumber`, `series`, `case`, `description`) with `package` (manufacturer package designation) and `qualification` (grade, e.g. AEC-Q100).
- **`thermal`** — extends PEAS `datasheetInfoThermal` (`operatingTemperature` as a `dimensionWithTolerance` min/max in degC) with `thermalResistanceJunctionAmbient` (K/W) and `maximumPowerDissipation` (W).
- **`mechanical`** — extends PEAS `datasheetInfoMechanical` (length/width/height/diameter/weight/shapeType/assemblyType) with `case`.

## Shared electrical cores (`utils.json`)

- **`supply`** — `minimumSupplyVoltage` / `maximumSupplyVoltage` (total span (V+)−(V−)), `dualSupply` (bool: bipolar-capable), `quiescentCurrentPerChannel`, `shutdownSupplyCurrent`. Device total quiescent current = `quiescentCurrentPerChannel × numberOfChannels` (derived, not stored).
- **`amplifierCommon`** (base for the linear-amplifier families) — `numberOfChannels`, `inputOffsetVoltage`, `inputOffsetVoltageDrift` (V/K), `inputBiasCurrent`, `inputOffsetCurrent`, `commonModeRejectionRatio` (dB), `powerSupplyRejectionRatio` (dB), `slewRate` (V/s), `voltageNoiseDensity` (V/√Hz) + `voltageNoiseDensityFrequency`, `supply`.
- **`switchCore`** (base for `analogSwitch` + `multiplexer`) — `onResistance`, `onResistanceFlatness`, `onResistanceMatch` (three distinct specs), `offLeakageCurrent`, `chargeInjection` (C), `turnOnTime`, `turnOffTime`, `offIsolation` (dB), `crosstalk` (dB), `bandwidth`, `signalRange` (`dimensionWithTolerance`), `controlLogic` (`logicFamily`), `supply`.
- **`converterDynamics`** (used by `adc` + `dac`) — `signalToNoiseRatio`, `totalHarmonicDistortion`, `spuriousFreeDynamicRange` (all dB) + `measurementFrequency`. **SINAD and ENOB are derived, never stored.**
- **`gainPoint`** — `{ gain, value }`, one sample of a gain-dependent spec.
- Enums: **`logicFamily`** (CMOS / TTL / TTL/CMOS), **`digitalInterface`** (SPI / I2C / parallel / microwire / pinStrap / serialLVDS / parallelLVDS / JESD204), **`referenceSource`** (internal / external / supply), **`inputType`** (singleEnded / differential / pseudoDifferential).

## Per-family `electrical`

**operationalAmplifier** = `amplifierCommon` + `gainBandwidthProduct` (≡ unity-gain BW for VFA), `openLoopGain` (dB), `outputCurrent`, `architecture` (generalPurpose/precision/lowNoise/lowPower/highSpeed/zeroDrift/currentFeedback/fullyDifferential/transimpedance), `inputStage` (CMOS/JFET/bipolar/BiCMOS), `railToRailInput`, `railToRailOutput`, `unityGainStable` (+ `minimumStableGain`), `hasShutdown`.

**comparator** (standalone) = `numberOfChannels`, `propagationDelay` (req) + `propagationDelayOverdrive` + `propagationDelayLoadCapacitance`, `inputOffsetVoltage`, `inputBiasCurrent`, `hysteresis` (absent = none), `outputStage` (pushPull/openDrain/openCollector/complementary), `commonModeVoltageRange`, `type` (generalPurpose/lowPower/highSpeed/precision/window), `hasLatch`, `supply`.

**instrumentationAmplifier** = `amplifierCommon` + `gainSetMethod` (req: singleResistor/fixed/pinProgrammable/digitallyProgrammable), `gainEquationConstant` (k in G=1+k/R_G), `minimumGain`/`maximumGain`, `bandwidthVsGain[]` & `commonModeRejectionRatioVsGain[]` (`gainPoint` arrays), `outputOffsetVoltage`, `inputCommonModeVoltageRange`, `architecture` (threeOpAmp/twoOpAmp/currentFeedback/zeroDrift).

**differenceAmplifier** = `amplifierCommon` + `gain` (req, fixed), `gainConfigurability` (fixed/pinSelectable), `gainOptions[]`, `gainError` (fraction), `gainErrorDrift` (1/K), `bandwidth`, `inputCommonModeVoltageRange` (may exceed rails), `inputOverloadVoltage`, `outputType` (singleEnded/differential).

**programmableGainAmplifier** = `amplifierCommon` + `gainValues[]` (req), `gainProgression` (binary/decade/scope/arbitrary), `digitalInterface[]`, `inputMultiplexerChannels`, `bandwidthVsGain[]`, `commonModeRejectionRatioVsGain[]`, `architecture` (opAmpPga/instrumentationPga).

**buffer** = `amplifierCommon` + `bufferType` (openLoopBuffer/closedLoopBuffer/lineDriver/videoBuffer/differentialDriver/powerBuffer), `bandwidth`, `gainConfiguration` (fixedUnity/fixedGain/adjustableGain) + `fixedGainValue`, `continuousOutputCurrent`, `peakOutputCurrent`, `outputImpedance`, `adjustableCurrentLimit`, `outputType`, `thermalShutdown`.

**sampleHold** = `amplifierCommon` + `subType` (sampleAndHold/trackAndHold), `acquisitionTime` + `acquisitionAccuracy`, `droopRate` (V/s), `holdCapacitor` (internal/external) + `holdCapacitance`, `apertureDelay`, `apertureJitter`, `holdStep`, `feedthrough` (dB), `bandwidth`, `gain`, `switchTechnology` (CMOS/FET/diodeBridge/bipolar).

**analogSwitch** = `switchCore` + `switchConfiguration` (req: SPST-NO/SPST-NC/SPDT/DPST/DPDT/SP3T/SP4T/DP3T/DP4T/4PST), `numberOfSwitches`, `totalHarmonicDistortion` (dB).

**multiplexer** = `switchCore` + `multiplexerConfiguration` (req, e.g. "8:1"), `numberOfChannels`, `signalConfiguration` (`inputType`), `breakBeforeMakeTime`, `transitionTime`.

**adc** = `resolution` (req, bits) + `architecture` (req: SAR/deltaSigma/pipeline/flash/dualSlope/folding), `sampleRate`, `numberOfChannels`, `inputType`, `integralNonlinearity`/`differentialNonlinearity` (LSB), `offsetError`/`gainError`, `dynamics` (`converterDynamics`, high-speed parts), `effectiveResolution` (precision ΔΣ parts), `fullScaleRange`, `hasProgrammableGainAmplifier`, `referenceSource` + `referenceVoltage`, `digitalInterface[]`, `supply` (analog rail) + `digitalSupplyVoltage`, `powerConsumption`.

**dac** = `resolution` (req) + `architecture` (req: stringDac/r2r/multiplying/sigmaDelta/segmented/currentSteering), `numberOfChannels`, `outputType` (voltageBuffered/voltageUnbuffered/current), `settlingTime`, `updateRate`, `integralNonlinearity`/`differentialNonlinearity`, `guaranteedMonotonic`, `gainError`, `dynamics`, `referenceSource` + `referenceVoltage`, `digitalInterface[]`, `supply` + `digitalSupplyVoltage`, `powerConsumption`, `hasEeprom`.

## Inputs / outputs

`inputs.designRequirements` = PEAS `designRequirementsBase` + `deviceType` (req, one of the 11 families) + generic seeds: `minimumChannels`, `supplyVoltage` (`dimensionWithTolerance`), `maximumQuiescentCurrent`, `maximumInputOffsetVoltage`, `minimumBandwidth`, `minimumResolution`.

`outputs` (each block wraps PEAS `outputBase` `{origin, methodUsed}`): `powerDissipation` (`quiescentLoss`/`outputLoss`/`totalLoss`), `thermal` (`junctionTemperature`, `passes`), `selection` (`supplyVoltageMargin`, `channelsSatisfied`, `passes`).
