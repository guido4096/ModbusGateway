from pathlib import Path
import subprocess


def run(compiler, src, temp, build):
    tests = Path(__file__).resolve().parent
    build(compiler, src, tests / 'transport.cpp', temp / 'transport',
          include_dirs=[tests / 'stubs'], production_transport=True,
          flags=['-pthread', '-fsanitize=address,undefined', '-fno-omit-frame-pointer'])
    for name in ('schema_device', 'diagnostics'):
        build(compiler, src, tests / (name + '.cpp'), temp / name,
              flags=['-pthread', '-fsanitize=address,undefined', '-fno-omit-frame-pointer'])
    invalid = temp / 'invalid_schema.cpp'
    invalid.write_text('#include "modbus/device.h"\n'
                       'struct Bad : modbus_gateway::SchemaApi<Bad> {\n'
                       'enum RegisterId { a, last };\n'
                       'inline static constexpr auto metadata = modbus_gateway::makeDeviceSchema<RegisterId>();\n'
                       '}; modbus_gateway::Device<Bad> invalid;\n')
    result = subprocess.run([compiler, '-std=c++17', '-fsyntax-only', '-I', str(src),
                             '-I', str(tests.parent / 'stubs'), str(invalid)],
                            text=True, capture_output=True)
    if result.returncode == 0 or 'Invalid device register schema' not in result.stderr:
        raise RuntimeError('Invalid schema must fail to instantiate Device')
