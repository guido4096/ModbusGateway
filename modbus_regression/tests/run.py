#!/usr/bin/env python3
"""Run the selected implementation tests without concrete device dependencies."""
import argparse
import pathlib
import runpy
import shutil
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
TESTS = ROOT / "tests"
STUBS = ROOT / "stubs"


def build(compiler, src, test, output, *, sources=(), flags=()):
    test = pathlib.Path(test)
    if not test.is_absolute():
        test = TESTS / test
    command = [compiler, '-std=c++17', '-O1', '-Wall', '-Wextra',
               '-UNDEBUG', '-I', str(STUBS), '-I', str(src), *flags,
               str(test), *(str(path) for path in sources), '-o', str(output)]
    completed = subprocess.run(command, text=True, capture_output=True)
    if completed.returncode:
        print(completed.stdout + completed.stderr, file=sys.stderr)
        raise RuntimeError(f"Compilation failed: {test}")
    subprocess.run([str(output)], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project-src', type=pathlib.Path, default=pathlib.Path('src'),
                        help='Gateway source directory (default: src)')
    src = parser.parse_args().project_src.resolve()
    compiler = shutil.which('clang++' if sys.platform == 'darwin' else 'g++')
    if not compiler:
        compiler = shutil.which('clang++') or shutil.which('g++')
    if not compiler:
        parser.error('C++17 compiler not found (clang++ or g++)')
    with tempfile.TemporaryDirectory(prefix='modbus-regression-') as path:
        temp = pathlib.Path(path)
        for suite in ('devices', 'plugin'):
            runner = ROOT / suite / 'run.py'
            if not runner.is_file():
                parser.error(f'Missing test runner: {runner}')
            runpy.run_path(str(runner))['run'](compiler, src, temp, build)
    print('PASS: selected gateway plugin native regression suite')


if __name__ == '__main__':
    try:
        main()
    except (RuntimeError, subprocess.CalledProcessError, ValueError) as exc:
        print(f'FAIL: {exc}', file=sys.stderr)
        sys.exit(1)
