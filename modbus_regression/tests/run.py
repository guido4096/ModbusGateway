#!/usr/bin/env python3
"""Compile and exercise the current two-device gateway using a native C++17 compiler."""
import argparse
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
TESTS = ROOT / "tests"
STUBS = ROOT / "stubs"


def conversion_body(source):
    """Extract the real method definition; avoid compiling networking dependencies."""
    pattern = r"void\s+(?:modbus_gateway::)?ConvertEM24_E1ToWattNode::CopyDataFromMasterToSlave\s*\(\s*\)"
    match = re.search(pattern, source)
    if not match:
        raise ValueError("Cannot find CopyDataFromMasterToSlave() in project converter")
    start = source.find('{', match.end())
    if start < 0:
        raise ValueError("Converter has no body")
    depth = 0
    for index in range(start, len(source)):
        if source[index] == '{':
            depth += 1
        elif source[index] == '}':
            depth -= 1
            if depth == 0:
                return source[match.start():index + 1]
    raise ValueError("Converter body has unmatched braces")


def build(compiler, src, test, output, includes):
    command = [compiler, '-std=c++17', '-O1', '-Wall', '-Wextra',
               '-UNDEBUG', '-I', str(STUBS), '-I', str(includes), '-I', str(src),
               str(TESTS / test), str(src / 'em24_e1.cpp'), str(src / 'wattnode.cpp'),
               '-o', str(output)]
    completed = subprocess.run(command, text=True, capture_output=True)
    if completed.returncode:
        print(completed.stdout + completed.stderr, file=sys.stderr)
        raise RuntimeError(f"Compilation failed: {test}")
    subprocess.run([str(output)], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project-src', type=pathlib.Path, default=pathlib.Path('src'),
                        help='Gateway source directory (default: src)')
    args = parser.parse_args()
    src = args.project_src.resolve()
    required = ('definitions.h', 'em24_e1.h', 'em24_e1.cpp', 'wattnode.h',
                'wattnode.cpp', 'convert_em24_e1_to_wattnode.cpp')
    missing = [name for name in required if not (src / name).is_file()]
    if missing:
        parser.error(f"Missing source files in {src}: {', '.join(missing)}")
    compiler = shutil.which('clang++' if sys.platform == 'darwin' else 'g++')
    if not compiler:
        compiler = shutil.which('clang++') or shutil.which('g++')
    if not compiler:
        parser.error('C++17 compiler not found (clang++ or g++)')
    body = conversion_body((src / 'convert_em24_e1_to_wattnode.cpp').read_text())
    # A two-device master is deliberately incompatible with the optimizer pipeline.
    if re.search(r'\b(?:Optimizer|SolarEdge|Alfen)\b', body):
        parser.error('Converter still references optional devices; use the two-device master source')
    with tempfile.TemporaryDirectory(prefix='modbus-regression-') as path:
        temp = pathlib.Path(path)
        build(compiler, src, 'schema_codec.cpp', temp / 'schema_codec', temp)
        (temp / 'conversion_body.inc').write_text(body + '\n')
        build(compiler, src, 'conversion.cpp', temp / 'conversion', temp)
    print('PASS: EM24_E1/WattNode native regression suite')


if __name__ == '__main__':
    try:
        main()
    except (RuntimeError, subprocess.CalledProcessError, ValueError) as exc:
        print(f'FAIL: {exc}', file=sys.stderr)
        sys.exit(1)
