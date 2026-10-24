#!/usr/bin/env python3
"""Native regression suite; no firmware is flashed and platformio.ini is untouched."""
import csv, io, pathlib, re, subprocess, sys, tempfile, struct, argparse
ROOT=pathlib.Path(__file__).resolve().parents[1]
OLD=ROOT/'baseline'; NEW=ROOT/'new'; STUB=ROOT/'stubs'
COMPILER='clang++' if sys.platform=='darwin' else 'g++'
SPECS=[('alfen','Alfen'),('em24_e1','EM24_E1'),('optimizer','Optimizer'),('solaredge','SolarEdge'),('wattnode','WattNode')]
DT={'float32':(0,2),'int16':(1,1),'uint16':(2,1),'int32':(3,2),'uint32':(4,2)}
SCALE={'none':1,'ten':10,'hundred':100,'thousand':1000}
# Match the original WattNode/EM24/Alfen/etc block header and original field initializer.
BLOCK=re.compile(r'\{\s*"([^"\n]+)"\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*,\s*\{')
FIELD=re.compile(r'\{\s*(\w+)\s*,\s*DataType::(\w+)\s*,\s*"([^"\n]*)"\s*,\s*"([^"\n]*)"\s*,\s*Scaling::(\w+)\s*,\s*Value::_\w+\(([^)]*)\)\s*(?:,\s*(true|false))?\s*\}')
def legacy_rows(path, name):
    text=path.read_text(); matches=list(BLOCK.finditer(text)); rows=[]
    for j,m in enumerate(matches):
        block,base=m.group(1),int(m.group(2),0); end=matches[j+1].start() if j+1<len(matches) else len(text)
        chunk=text[m.end():end]; address=base
        for f in FIELD.finditer(chunk):
            ident,typ,desc,unit,scale,default,high=f.groups(); typecode,width=DT[typ]
            default=default.strip()
            if default in ('rtuServerId','serialNumber'): bits=None
            else:
                try:
                    value=int(default,0) if typ!='float32' else float(default)
                    bits=(struct.unpack('>I',struct.pack('>f',value))[0] if typ=='float32' else value & 0xffffffff)
                except ValueError: raise RuntimeError(f'unhandled legacy default {default} in {path}')
            rows.append((name,block,base,len(rows),address,typecode,SCALE[scale],1 if high=='true' else 0,desc,unit,bits))
            address+=width
    return rows

def build_test(src,output):
    cmd=[COMPILER,'-std=c++17','-O1','-Wall','-Wextra','-I',str(STUB),'-I',str(NEW),str(src),str(NEW/'wattnode.cpp'),str(NEW/'alfen.cpp'),str(NEW/'em24_e1.cpp'),str(NEW/'optimizer.cpp'),str(NEW/'solaredge.cpp'),'-o',str(output)]
    result=subprocess.run(cmd, text=True,capture_output=True)
    if result.returncode:
        print(result.stdout+result.stderr)
        raise RuntimeError('Native C++ compilation failed')

def main():
    global NEW
    parser=argparse.ArgumentParser(description='Compare original register map with native-compiled new schema; never flashes.')
    parser.add_argument('--project-src',type=pathlib.Path,help='Use headers/cpps in your current PlatformIO src/ instead of bundled snapshot')
    args=parser.parse_args()
    if args.project_src: NEW=args.project_src.resolve()
    print('Candidate new schema directory:', NEW)
    print('Native C++17 regression tests (no device needed)')
    with tempfile.TemporaryDirectory() as tmp:
        dump=pathlib.Path(tmp)/'schema_dump'; build_test(ROOT/'tests/schema_dump.cpp',dump)
        res=subprocess.run([str(dump)],check=True,text=True,capture_output=True)
        actual={}
        for row in csv.reader(io.StringIO(res.stdout),delimiter='\t'):
            key=row[0]; actual.setdefault(key,[]).append((row[0],row[1],int(row[2]),int(row[3]),int(row[4]),int(row[5]),int(row[6]),int(row[7]),row[8],row[9],int(row[10])))
        all_count=0; discrepancies=[]
        for filename,schema in SPECS:
            base=legacy_rows(OLD/(filename+'.cpp'),filename)
            # ESP32 is little-endian: old Value.w1 denotes the LOW word.
            # Legacy read float32 uses `_wordorder` to choose between word orderings;
            # int32/uint32 toFloat32() ignores it and always treats w1 as low.
            # For these supplied schemas every 32-bit effective read is the inverse
            # of the metadata boolean (all int32/uint32 flags are true).
            base = [row[:7] + (1 - row[7],) + row[8:] if row[5] in (0,3,4) else row
                    for row in base]
            got=actual.get(filename,[])
            if len(base)!=len(got): discrepancies.append(f'{schema}: original {len(base)} registers, new {len(got)}')
            for idx,(b,g) in enumerate(zip(base,got)):
                for col,field in enumerate(['device','block','block base','register id','address','type','scaling','word order','description','unit','default bits']):
                    if b[col] is not None and b[col]!=g[col]: discrepancies.append(f'{schema} register {idx} {field}: original={b[col]!r}, new={g[col]!r}')
            print(f'  {schema}: {len(base)} original vs {len(got)} compiled metadata records')
            all_count+=len(base)
        if discrepancies:
            for d in discrepancies[:40]:print('FAIL:',d)
            print(f'REGRESSION: {len(discrepancies)} metadata mismatches (continuing codec tests)')
        print(f'CHECKED: {all_count} register descriptions against original sources')
        binary=pathlib.Path(tmp)/'codec_test';build_test(ROOT/'tests/codec_test.cpp',binary)
        subprocess.run([str(binary)],check=True)
        script=subprocess.run([sys.executable,str(ROOT/'tests/conversion_audit.py'),str(NEW/'convert_em24_e1_to_wattnode.cpp')],text=True)
        if script.returncode: raise AssertionError('Conversion source audit failed')
    if discrepancies: raise AssertionError(f'{len(discrepancies)} metadata mismatches')
    print('PASS: native offline regression gates completed')

if __name__=='__main__':
    try: main()
    except Exception as ex: print('FAIL:',ex,file=sys.stderr);sys.exit(1)
