# Native regression tests

From the project root:

```sh
python3 modbus_regression/tests/run.py --project-src src
```

Requires Python 3 and a native C++17 compiler (`clang++` or `g++`). The shared
runner in `tests/run.py` runs `devices/run.py` for the shared catalog, then
`plugin/run.py`, which chooses the tests
and production sources for the selected implementation. No device names or
converter paths are hardcoded in the shared runner.

`plugin/` contains register/schema checks, explicit reference Modbus words and
conversion scenarios for the current implementation. It may differ between
branches. Tests compile production converter headers, source, helpers and codecs;
`stubs/modbus/` substitutes transport owners with mutex-protected devices.
Lifecycle tests also compile the production plugin entry point with simulated
HTTP, serial and network transports to check startup, scheduling and route behavior.

Native tests do not cover hardware transports, Ethernet, OTA or real-time
scheduling. Also run `pio run -e T-ETH-POE-PRO` for the firmware toolchain.

`devices/` tests reusable manufacturer-defined schemas on every branch.
Availability in this catalog is independent of the devices selected by a plugin.
