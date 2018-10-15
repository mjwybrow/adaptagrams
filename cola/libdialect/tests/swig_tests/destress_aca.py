from pkg.adaptagrams import *

g = buildGraphFromTglfFile("../graphs/special/X_4.tglf")
#print g.getMaxDegree()
#print g.getNumNodes()
aca = ACALayout(g)
aca.layout()
g.updateNodesFromRects()
aca.updateSepMatrix()
t1 = g.writeTglf()
with open('./output/X_4_aca.tglf', 'w') as f:
    f.write(t1)

opts = ColaOptions()
g.destress(opts)
t2 = g.writeTglf()
with open('./output/X_4_aca_destress.tglf', 'w') as f:
    f.write(t2)

