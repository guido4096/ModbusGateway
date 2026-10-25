from pathlib import Path


def run(compiler, src, temp, build):
    tests = Path(__file__).resolve().parent
    sources = sorted((src / 'devices').glob('*.cpp'))
    for name in ('schema_codec', 'alfen_solaredge'):
        build(compiler, src, tests / (name + '.cpp'), temp / name, sources=sources)
