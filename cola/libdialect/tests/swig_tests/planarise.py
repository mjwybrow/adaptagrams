from pkg.adaptagrams import *

g = buildGraphFromTglfFile("../graphs/special/bundlecross.tglf")
hola_opts = HolaOpts()
hola_opts.routingScalar_shapeBufferDistance = 1/8.0
lor = LeaflessOrthoRouter(g, hola_opts)
lor.route()
op = OrthoPlanariser(g);
P = op.planarise();
t = P.writeTglf()
with open('./output/planarise.tglf', 'w') as f:
    f.write(t)


