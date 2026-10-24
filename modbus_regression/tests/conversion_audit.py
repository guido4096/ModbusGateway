import pathlib,re,sys
BASE=pathlib.Path(__file__).resolve().parents[1]
old=(BASE/'baseline/convert_em24_e1_to_wattnode.cpp').read_text()
new=pathlib.Path(sys.argv[1]).read_text() if len(sys.argv)>1 else (BASE/'new/convert_em24_e1_to_wattnode.cpp').read_text()
def exprs(code,legacy):
  out=[]
  for line in code.splitlines():
    codepart=line.split('//')[0].strip()
    if not codepart.startswith('wattnode.set'):continue
    if legacy:
      codepart=re.sub(r'wattnode\.setFloatValue\(WattNode::(\w+),',r'wattnode.set<WattNode::\1>(',codepart)
      codepart=re.sub(r'meter\.getFloatValue\(EM24_E1::(\w+)\)',r'engineering<EM24_E1::\1>(meter)',codepart)
    out.append(''.join(codepart.split()))
  return out
a=exprs(old,True);b=exprs(new,False)
if a!=b:
  print('Conversion expressions differ (first mismatches):')
  for i,(x,y) in enumerate(zip(a,b)):
    if x!=y:print('  index',i,'old:',x,'new:',y)
  if len(a)!=len(b):print('  expression counts:',len(a),len(b))
  raise SystemExit(1)
print(f'PASS: {len(a)} WattNode assignment expressions structurally preserved')
print('NOTE: Structural audit is not runtime conversion equivalence testing.')
