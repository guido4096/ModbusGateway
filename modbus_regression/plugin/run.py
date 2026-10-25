from pathlib import Path


def run(compiler, src, temp, build):
    tests = Path(__file__).resolve().parent
    plugin = src / 'plugin'
    devices = [src / 'devices' / (name + '.cpp')
               for name in ('em24_e1', 'wattnode', 'alfen', 'solaredge')]
    devices.append(plugin / 'devices/optimizer.cpp')
    conversion = plugin / 'convert_em24_e1_to_wattnode.cpp'
    power = plugin / 'battery_power.cpp'
    for name in ('conversion', 'optional_schema', 'description_test'):
        build(compiler, src, tests / (name + '.cpp'), temp / name,
              sources=[*devices, conversion, power, tests / 'native_inputs.cpp'])
    build(compiler, src, tests / 'battery_conversion.cpp', temp / 'battery_conversion',
          sources=[*devices, conversion, power])
    build(compiler, src, tests / 'lifecycle.cpp', temp / 'lifecycle',
          sources=[*devices, conversion, power, plugin / 'plugin.cpp'],
          flags=['-DREMOTE="127.0.0.1"', '-DTCP_PORT=502', '-DTCP_SERVER_ID=1',
                 '-DRTU_SERVER_ID=2', '-DSERIAL_NUMBER=0x12345678u',
                 '-DREMOTE_ALFEN="127.0.0.2"', '-DTCP_ALFEN_PORT=502', '-DTCP_ALFEN_SERVER_ID=1',
                 '-DREMOTE_SOLAREDGE="127.0.0.3"', '-DTCP_SOLAREDGE_PORT=502', '-DTCP_SOLAREDGE_SERVER_ID=1'])
