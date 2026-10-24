# ModbusGateway offline regression tests

This is the **single, current guide** for the Modbus migration regression suite. The tests run natively on macOS from VS Code or Terminal, **without flashing an ESP32, opening serial ports, or changing `platformio.ini`**. They require Python 3 and Apple `clang++` (install Xcode Command Line Tools with `xcode-select --install` if necessary).

## Run against your actual PlatformIO sources

Unzip `modbus_regression/` into your project root, beside `src/` and `platformio.ini`. From the VS Code terminal in that directory:

```sh
python3 modbus_regression/tests/run.py --project-src src
```

You can also run the packaged historical snapshot, but **it is older than the corrected word-order schemas and is expected to fail the current metadata checks**. Use `--project-src src` to validate your current code:

```sh
python3 modbus_regression/tests/run.py
```

Run **only** the executable conversion comparison:

```sh
python3 modbus_regression/tests/conversion_equivalence.py src
```

The scripts return a nonzero exit status when an unaccepted difference or compilation failure occurs. The shipped historical `new/` snapshot currently produces 124 word-order metadata mismatches; this is **not a passing reference fixture** and is kept unchanged in this documentation-only revision. They compile temporary native binaries; they do **not** upload firmware. After the tests pass, you can independently compile the ESP32 target (also without uploading):

```sh
pio run -e T-ETH-POE-PRO
```

## Contents and sources of truth

- `baseline/`: archived original, pre-migration device register tables and conversion source supplied for the comparison. Preserve these files; do not overwrite them with migrated versions.
- `new/`: a bundled migrated-source snapshot used when `--project-src` is omitted. **Your current `src/` is the actual candidate** when the option is supplied.
- `stubs/`: small native Arduino compatibility shim; this is not a real ESP32 or eModbus runtime.
- `tests/`: Python orchestration, C++ codec/schema tests, structural conversion audit, and executable conversion fixtures.

## What the full command checks

1. **Compile-time schema equivalence:** compiles the migrated C++17 schemas, exports metadata and compares the five device maps against original register tables: Alfen (13), EM24_E1 (52), Optimizer (1), SolarEdge (5), WattNode (110), **181 registers total**. The comparison covers IDs, addresses, block membership, widths (via types), data types, scaling, effective 32-bit word order, labels, units, and fixed defaults. The legacy word-order flag is interpreted according to the old ESP32 little-endian union behavior; directly comparing flag values was an earlier test defect.
2. **Independent codec test vectors:** tests encoded 16-bit words for signed/unsigned numbers, float32, word order, selected defaults, and invalid-range rejection. Tests must use expected raw words, not just a write/read round trip.
3. **Structural conversion audit:** checks the **51 active WattNode assignments** in the old/new conversion sources for equivalent formulas after adapting old accessor names. This is a source-level guard, not a runtime proof.
4. **Executable conversion comparison:** builds native baseline and migrated conversion executables with simulated register access and identical deterministic inputs. Compares all **173 WattNode raw 16-bit words** for each scenario, including optimizer modes **0 (normal), 1 (PV surplus), and 2 (grid only)**, negative power and other configured cases.

## Known, intentionally accepted difference

**Scenario 5** includes fractional Alfen/SolarEdge power. The original float getter rounded float32 inputs to whole units; the new typed getter retains fractions. The executable comparison accepts a difference *only* in the strategy-adjusted WattNode power words at indices **10–17**, and prints `KNOWN DIFFERENCE` with the changed words. The other **five scenarios must match exactly**. This exception is narrowly scoped, not a claim of full behavioral equivalence.

The earlier migration also surfaced a WattNode `modbus_address` word-order metadata discrepancy (single-word `int16`, no wire-order effect). The bundled corrected snapshot retains the original flag for that field; if your own `src/` differs, the metadata comparison will report it.

## What passing does **not** prove

- The tests exercise conversion calculations through adapted mock `DataAccess`, not the original full `Device`/Modbus implementation, so they are not exhaustive proof of exact on-device behavior.
- They do not execute real asynchronous eModbus TCP/RTU callbacks, FC03/FC06/FC16 request/response traffic, threading, timers, Ethernet, OTA or rollback behavior.
- They are not a substitute for actual ESP32 measurements and protocol captures. You have separately verified the meter-to-SolarEdge, Alfen and optimizer paths on hardware.

Keep this suite in Git beside the firmware and run it before future schema/codec changes. To add scenarios, extend `tests/conversion_runner.cpp` and the corresponding conversion fixtures, then rerun the full command.
