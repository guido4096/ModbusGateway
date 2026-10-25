# Modbus gateway native regression tests

This suite checks the register schemas, Modbus wire encoding and
EM24_E1-to-WattNode conversion, including the three optimizer strategies.

Run from the PlatformIO project root:

```sh
python3 modbus_regression/tests/run.py --project-src src
```

Requires Python 3 and a native C++17 compiler (`clang++` or `g++`). It does not
flash the ESP32, change `platformio.ini`, or require Modbus network hardware.

- `tests/schema_codec.cpp`: EM24_E1/WattNode invariants, reference wire words,
  signed values, defaults, block coverage and invalid addresses. **Identical to master.**
- `tests/optional_schema.cpp`: Alfen, SolarEdge and Optimizer layouts and wire
  encodings, including signed values and battery power.
- `tests/conversion.cpp`: the EM24-to-WattNode examples from master, extended with
  strategy 1 (PV surplus), strategy 2 (grid only) and a return to strategy 0.
- `tests/run.py`: builds and executes native tests in a temporary directory.
- `stubs/Arduino.h`: minimal native definitions. **Identical to master.**

The conversion harness substitutes only transport owners with native
`Device` instances. It compiles `CopyDataFromMasterToSlave()` extracted from
the current project converter; conversion formulas are not duplicated.

These tests do not cover the transport layer, Ethernet, OTA, threading or the
ESP32 build. Use `pio run -e T-ETH-POE-PRO` separately.

Update reference assertions deliberately when device behavior changes.
