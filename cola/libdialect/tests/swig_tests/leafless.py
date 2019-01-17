from pkg.adaptagrams import *

g = buildGraphFromTglfFile("../graphs/random/v40e42.tglf")
peel(g)
g.destress()
opts = OrthoHubLayoutOptions()
ohl = OrthoHubLayout(g, opts)
ohl.layout()
hola_opts = HolaOpts()
lor = LeaflessOrthoRouter(g, hola_opts)
lor.route()
t = g.writeTglf()
with open('./output/leaflessroute.tglf', 'w') as f:
    f.write(t)


