from pathlib import Path


def run(compiler, src, temp, build):
    tests = Path(__file__).resolve().parent
    plugin = src / 'plugin'
    devices = [src / 'devices' / (name + '.cpp')
               for name in ('em24_e1', 'wattnode')]
    conversion = plugin / 'convert_em24_e1_to_wattnode.cpp'
    build(compiler, src, tests / 'conversion.cpp', temp / 'conversion',
          sources=[*devices, conversion])
    build(compiler, src, tests / 'lifecycle.cpp', temp / 'lifecycle',
          sources=[*devices, conversion, plugin / 'plugin.cpp'],
          flags=['-DREMOTE="127.0.0.1"', '-DTCP_PORT=502', '-DTCP_SERVER_ID=1',
                 '-DRTU_SERVER_ID=2', '-DSERIAL_NUMBER=0x12345678u'])
