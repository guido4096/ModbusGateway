# EM24_E1 → WattNode native regression tests

This suite tests the register schemas, Modbus wire encoding and the
EM24_E1-to-WattNode conversion. 

From the PlatformIO project root:

```sh
python3 modbus_regression/tests/run.py --project-src src
```

Requires Python 3 and a native C++17 compiler (`clang++` or `g++`). It does not
flash the ESP32, change `platformio.ini`, or require eModbus/network hardware.

- `tests/schema_codec.cpp`: schema invariants, register counts, fixed reference
  Modbus words, signed values, defaults, block coverage, and invalid addresses.
- `tests/conversion.cpp`: physical-unit examples and wire-word checks, compiling
  the actual conversion method extracted from `src/convert_em24_e1_to_wattnode.cpp`.
- `tests/run.py`: builds in a temporary directory, runs both binaries, and
  cleans up afterward.
- `stubs/Arduino.h`: minimal native definitions for schema compilation.

The conversion harness substitutes only transport owners with mutex-protected
`Device` instances; it compiles the method body from the current project rather
than copying its formulas. This requires the two-device converter on `master`.

These tests cover native schema/conversion behavior, not Ethernet, RTU, OTA,
real-time scheduling, or the ESP32 toolchain. Run `pio run -e T-ETH-POE-PRO`
separately for the firmware build.

If register definitions or conversion behavior intentionally change, update
explicit assertions in the tests and review the changed wire values.
