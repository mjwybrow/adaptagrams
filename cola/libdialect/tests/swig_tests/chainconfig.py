from pkg.adaptagrams import *

g = buildGraphFromTglfFile("../graphs/special/chainconfig01.tglf")
g.getSepMatrix().setCardinalOP(0, 1, CardinalDir_SOUTH)
g.getSepMatrix().setCardinalOP(16, 15, CardinalDir_NORTH)
chains = buildAllChainsInGraph(g)
chain = chains[0]
chain.takeShapeBasedConfiguration()
opts = ColaOptions()
g.project(opts, XDIM)
g.project(opts, YDIM)
g.destress()
with open('./output/chainconfig.tglf', 'w') as f: f.write(g.writeTglf())

