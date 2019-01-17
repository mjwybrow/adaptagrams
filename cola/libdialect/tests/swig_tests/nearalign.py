from pkg.adaptagrams import *

g = buildGraphFromTglfFile("../graphs/special/nearalign01.tglf")
opts = HolaOpts()
ignore = DialectNodeLookup()
atab = AlignmentTable(g, ignore)
n = doNearAlignments(g, atab, ignore, opts)
assert n == 2
t = g.writeTglf()
with open('./output/nearalign.tglf', 'w') as f:
    f.write(t)

