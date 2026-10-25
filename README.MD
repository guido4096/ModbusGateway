<h1 align = "center">ESP32 (LilyGO T-POE-PRO) Modbus Gateway between SolarEdge/RTU and EM24/TCP</h1>

This is a gateway to connect a SolarEdge inverter to your own energy meter.

SolarEdge can only communicate to a specific set of energy meters, sold by SolarEdge.
These meters are expensive and it adds clutter to your electricity cabinet when you
already have an energy meter.

This projects aims to reuse your own energy meter. This projects mimics a WattNode
meter, which is compatible with SolarEdge and well documented. It retrieves the
energy and power values from a Carlo Gavazzi EM24 energy meter, which it then translates
to the WattNode definitions.

It not only converts the different energy meter definitions, it also converts the physical
layer between Modbus-RTU and Modbus-TCP.

The SolarEdge inverter connects to the LilyGO ETH-POE-PRO through Modbus-RTU over RS-485.
The LilyGO-POE-PRO connects to a Carlo Gavazzi EM24 through Modbus-TCP.
The gateway translates the registers and values as needed.

## 1 PlatformIO Quick Start <Recommended>

1. Install [Visual Studio Code](https://code.visualstudio.com/) and [Python](https://www.python.org/)
2. Search for the `PlatformIO` plugin in the `VisualStudioCode` extension and install it.
3. After the installation is complete, you need to restart `VisualStudioCode`
4. After restarting `VisualStudioCode`, select `File` in the upper left corner of `VisualStudioCode` -> `Open Folder` -> select the `LilyGO-ModbusGateway` directory
5. Wait for the installation of third-party dependent libraries to complete
7. Click the (✔) symbol in the lower left corner to compile
8. Connect the board to the computer USB (If there is no onboard downloader, USB2TTL must be connected)
9. Click (→) to upload firmware
10. Click (plug symbol) to monitor serial output


## Source layout

```text
src/
  main.cpp                 Board, networking, HTTP host and OTA
  utilities.h              Board pin definitions
  application/plugin.h     Device-independent plugin contract
  modbus/                  Shared schema, codec, storage and transport code
  devices/                 Reusable manufacturer-defined schemas and factories
  plugin/                  Complete implementation selected by this branch
    plugin.cpp             Device ownership, startup, polling and web handlers
    devices/               Plugin-specific schemas (such as Optimizer)
    convert_*.h / .cpp     Device-specific conversion
```

`master` implements the EM24-to-WattNode gateway. `optimize-battery-usage`
implements the gateway with additional battery optimization. Both are complete
plugins; neither implementation is hardcoded in the host or test runner.

## Creating another gateway

1. Create a branch from `master`.
2. Replace `src/plugin/` with your device selection, transport configuration and conversion.
   Reuse schemas from `src/devices/`; keep custom application schemas in the plugin.
3. Implement the three functions declared in `src/application/plugin.h`:
   `setup(WebServer&)`, `loop(unsigned long)` and `appendRootLinks(String&)`.
4. Replace the matching tests and source list in `modbus_regression/plugin/`.
5. Run the native tests and firmware build.

The host calls `setup` after networking is ready and before the HTTP server
starts. A plugin registers its own routes, starts its transports and initializes
its timers. `loop` performs its own polling and conversion and returns true when
the host should yield. The root page includes plugin-supplied menu links; `/info`
and the HTTP fallback belong to the host.

There is exactly one statically linked plugin, compiled automatically from
`src/plugin/`; no activation macro or runtime registration is required. A plugin
may use entirely different devices or transports without changing `main.cpp`,
`platformio.ini` or `src/modbus/`. Configure installation-specific addresses and
IDs through your plugin and existing local `secrets.ini` build flags.

Shared host/infrastructure improvements can be made on `master` and merged into
other branches. Concrete plugin implementations may intentionally diverge.

## Validation

```sh
python3 modbus_regression/tests/run.py --project-src src
pio run -e T-ETH-POE-PRO
```

The shared native runner delegates to `modbus_regression/plugin/run.py`. That
implementation-specific runner selects devices and compiles production conversion
code with native transport-owner substitutes. Register definitions, word order,
scaling and default values remain subject to the same compile-time checks.

## Shared device catalog

`src/devices/` contains Alfen, SolarEdge, WattNode and EM24_E1 on both branches.
Plugins include the devices they need, for example `#include "devices/alfen.h"`.
The catalog provides schemas and factories; it opens no connections, registers
no endpoints and schedules no polling. The plugin creates the actual instances.
The application-defined Optimizer schema remains under `src/plugin/devices/`.

Shared catalog tests live in `modbus_regression/devices/` and run on every branch,
including for devices that its plugin does not use. Adding a reusable device does
not require a change to the host or existing plugins.
