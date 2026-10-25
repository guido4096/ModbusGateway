# ModbusGateway — Development Guidelines

## Project

ESP32 Arduino / PlatformIO project using C++17 and eModbus.

Repository: https://github.com/guido4096/ModbusGateway

Two branches have intentionally different scopes:

- `master` plugin: EM24_E1, WattNode and EM24-to-WattNode conversion only.
- `optimize-battery-usage` plugin: additionally uses Alfen, SolarEdge and Optimizer.

Both branches share manufacturer-defined devices in `src/devices/`. Availability
in the catalog does not activate a device: the plugin owns its use and transports.
Do not introduce optimizer behavior into the `master` plugin.

## Architecture

- `src/main.cpp` hosts board, network, HTTP and OTA services only.
- `src/application/plugin.h` is the device-independent plugin contract.
- Each branch supplies one complete implementation in `src/plugin/`, including
  transport instances, polling, conversion, web routes and plugin-specific schemas.
- Reusable manufacturer-defined schemas and factories live in `src/devices/`.
- Alfen, SolarEdge, WattNode and EM24_E1 are shared; Optimizer stays plugin-specific.
- Plugin code may intentionally differ or be duplicated between branches.
- Keep shared host and Modbus infrastructure independent of concrete devices.

- Register schemas are defined at compile time using `constexpr` metadata.
- Validate register layouts, addresses, IDs and widths at compile time.
- Runtime register values use fixed-size storage.
- Typed register access uses compile-time register IDs.
- Preserve explicit Modbus word order, data types, scaling and default values.
- Register descriptions must remain declarative and readable.
- Avoid unnecessary runtime allocation and template abstraction.
- Preserve existing interfaces unless there is a demonstrated reason to change them.

## Tests

- `modbus_regression/` tests the current implementation, not historical versions.
- Do not introduce `baseline/` or `new/` copies of source files.
- Keep common test infrastructure identical between branches.
- Shared device tests and their runner belong in `modbus_regression/devices/`.
- Implementation-specific tests and their runner belong in `modbus_regression/plugin/`.
- Add branch-specific tests only for branch-specific functionality.
- Test actual production conversion code, not duplicated implementations.
- Include explicit expected raw Modbus words, especially for 32-bit values and negative numbers.

Run:

```bash
python3 modbus_regression/tests/run.py --project-src src
pio run -e T-ETH-POE-PRO
```

## Coding conventions

- Preserve original copyright, license and attribution headers.
- Comments should explain current design decisions, invariants and non-obvious behavior.
- Do not include migration-history comments or explanations of obvious statements.
- Prefer straightforward, maintainable C++17.
- Avoid unnecessary boilerplate and redundant abstractions.
- Do not claim tests pass unless they have actually been executed.
- Keep a single README rather than creating successive documentation versions.

## Git workflow

- `master` deliberately has less functionality.
- Promote shared improvements selectively.
- Keep branch-specific code and tests isolated.
- Minimize differences between common files across branches.
- Review changes with `git diff` before committing.
