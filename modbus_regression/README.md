# Offline Modbus migration regression test — macOS / VS Code

This folder is self-contained and **does not flash**, upload, open serial ports, or modify `platformio.ini`.
The `baseline/` sources are copies of the uploaded **original** device declarations, not hand-reconstructed expected mappings. The `new/` files are a snapshot of the previous compile-time rewrite. Tests use your Mac's `clang++` and Python 3; no PlatformIO installation is required for this native stage.

## Quick start in VS Code

1. Unzip this folder into the **root of your PlatformIO project** as `modbus_regression/` (same level as `src/` and `platformio.ini`).
2. Open VS Code **Terminal > New Terminal** in the project root.
3. Run the tests against the **actual code you are about to flash**:

```sh
python3 modbus_regression/tests/run.py --project-src src
```

Or run the bundled snapshot without touching your project:

```sh
python3 modbus_regression/tests/run.py
```

The command exits with code 0 when checks pass, nonzero if any fail. On macOS you may need the Apple Command Line Tools (`xcode-select --install`) for `clang++`. Do not run `pio run --target upload` as part of these checks.

## What it does

1. Compiles the actual new schema C++17 code for **your Mac**, using a small stub only for basic Arduino symbols. It emits metadata from compiled `Schema::metadata` for the five devices.
2. Independently parses the original `alfen.cpp`, `em24_e1.cpp`, `optimizer.cpp`, `solaredge.cpp`, `wattnode.cpp` and compares all 181 register descriptions: block, address, type, scaling, word order, name, unit, and static defaults. Original dynamic WattNode defaults are checked separately by a runtime C++ test.
3. Runs native C++ tests against independently specified raw-word vectors: WattNode high-word-first, Alfen low-word-first, signed int32, Optimizer, bounds, atomic rejection of invalid writes, and serial-number default.
4. Checks that **51 uncommented WattNode assignments** in the original conversion logic have corresponding structurally equivalent expressions in the migrated source (normalizing only the typed getter syntax). This is **a source-level check, not proof of runtime equivalence**.

## Known detected difference

The *unmodified* snapshot initially failed at WattNode register ID 83 (`modbus_address`): original declaration omitted `_wordorder` (thus initialized `false`); migrated declaration explicitly says `true`. This is a single-word `int16` register, so the flag should not affect its Modbus bytes, but a strict source-to-source metadata test correctly reports the difference.

Fix in your new `src/wattnode.h` if you want exact metadata equivalence:

```cpp
{modbus_address, DataType::int16, Scaling::none, false, "Modbus Address", "", 0},
```

The `new/` snapshot inside this test package includes this **explicit, documented correction**, so that running the bundled snapshot can pass. Running with `--project-src src` still tests your actual unmodified files and will flag it until corrected.

## What these tests DO NOT establish

- That the **old and new converter implementations produce the same runtime values** for all optimizer strategies. The assignment audit checks formulas, but it cannot independently certify original numeric conversion behavior; for that we'd need a dual-build mock-client fixture or recorded original firmware samples.
- That eModbus callback parsing and FC03/FC06/FC16 responses are correct on the actual ESP32 libraries.
- That network/serial/OTA handling, concurrency, or rollback works on the ESP32.
- That the original union's word-order behavior always matches the new codecs on actual hardware. Treat wire samples captured from the old gateway as golden references whenever possible.

**Do not flash solely because these tests pass.** Your OTA recovery still needs a separate test or a verified way to reflash over USB.
