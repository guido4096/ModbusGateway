# ModbusGateway — Development Guidelines

## Project

ESP32 Arduino / PlatformIO project using C++17 and eModbus.

Repository: https://github.com/guido4096/ModbusGateway

Two branches have intentionally different scopes:

- `master`: EM24_E1, WattNode and EM24-to-WattNode conversion only.
- `optimize-battery-usage`: additionally supports Alfen, SolarEdge and Optimizer.

Do not introduce dependencies on the additional devices into `master`.

## Architecture

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
- Keep common tests identical between branches wherever possible.
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
