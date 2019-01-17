from pkg.adaptagrams import *

g = buildGraphFromTglfFile("../graphs/special/core_with_trees.tglf")
opts = OrthoHubLayoutOptions()
ohl = OrthoHubLayout(g, opts)
ohl.layout()
t = g.writeTglf()
with open('./output/core_with_trees_ohl.tglf', 'w') as f:
    f.write(t)

