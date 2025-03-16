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

## 2 RESOURCE

* [T-ETH-PRO POE Module datasheet](./datasheet/ETH-PRO-POE-DP5300-12V.pdf)

