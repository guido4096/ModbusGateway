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

## Modbus infrastructure behavior

Client data notifications are consumed with `consumeUpdated()` before conversion.
An update arriving during conversion remains pending for a subsequent pass.
Use `DataAccess` for device values; client/server storage is private.

Each client keeps up to 32 pending requests in fixed storage. A full context
table rejects new requests until a completion or error releases a slot. Tokens
are numeric IDs, so callbacks do not depend on the host pointer width. A client
exclusively owns its transport's data/error callback pair. Stop/drain the transport
before destroying its client; callbacks are detached during destruction.

Servers bind callbacks to their own instance. The same schema may be used by
multiple endpoints; an occupied transport/server-ID/function combination is
rejected and `registered()` reports false. Stop request processing before
destroying a server; its registered workers are then removed.

Statistics use atomic counters and serialized sampling. The log has a fixed
200-entry ring, synchronized reads/writes, and nonblocking wall-clock timestamps
with an uptime fallback before the clock is synchronized. Strings still allocate
their own contents.

Schemas retain `metadata.valid` and also expose the first `metadata.error` as a
`SchemaError`. `Device` rejects invalid schemas at compile time, including
unrepresentable addresses, offsets, lengths and invalid field properties.

`Client` and `Server` expose `getLog()`, `getStatistics()` and `logMessage()`
directly. These methods use diagnostics' own synchronization without taking the
register lock. DataAccess exclusively provides register access.

HTTP handlers collect register responses inside a short DataAccess scope, release
the guard, then send the response. Strategy handlers also release the guard
before writing preferences to flash. The response describes the register state
captured when the strategy was set.

## Reading the Modbus code

Start with a device header in `src/devices/` to see a concrete layout, then read
the public API of `Device`, `Client` or `Server`. The schema builder can be read
after those interfaces: its nested lambdas implement the compile-time layout,
rather than the runtime request flow.

| File | Responsibility | Useful entry points |
| --- | --- | --- |
| `register_schema.h` | Declare and validate immutable register layouts | `field`, `block`, `makeDeviceSchema` |
| `register_codec.h` | Convert raw values to and from Modbus words | `decodeTyped`, `encodeTyped`, `loadBits`, `storeBits` |
| `device.h` | Store words and validate address ranges | `get`, `set`, `readHoldingRegisters`, `writeHoldingRegisters` |
| `data_access.h` | Hold an owner's register lock for one scope | constructor and destructor |
| `client.h` | Schedule TCP reads and publish validated replies | `submit`, `handleData`, `validResponse` |
| `server.h` | Validate protocol requests and serve cached words | `processRequest`, `validateRequest`, `readRegisters`, `writeRegisters` |
| `diagnostics.h` | Synchronize counters and the log ring | `Statistics`, `Log` |
| `device_diagnostics.h` | Format register values and layout descriptions | `formatRegisterValue`, `formatDeviceValues`, `printDeviceDescription` |
| `definitions.h` | Collect the shared interfaces in one include | includes only |

`Schema` denotes a device layout type. A register template argument denotes an
enum ID known at compile time; `value_type<Register>` derives its C++ value type
from that layout. `typename` and the dependent-call `template` keyword are C++
syntax qualifiers, not extra runtime operations. Typed access returns raw native
values; engineering scaling is applied explicitly by conversion or diagnostics.

The two runtime flows are:

- Incoming request: server validation, register read/write under DataAccess, response.
- Outgoing read: client range validation, request reservation, transport enqueue,
  callback validation, register publication under DataAccess, update notification.

The request-table mutex protects tokens and slots; DataAccess protects register
words. Neither lock is held while enqueueing a transport request. Logs and
statistics own their separate synchronization.
