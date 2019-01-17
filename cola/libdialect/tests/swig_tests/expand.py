from pkg.adaptagrams import *

opts = HolaOpts()
# Build a graph
g = buildGraphFromTglfFile("../graphs/special/expand03.tglf")
# Peel.
trees = peel(g)
# Do an ortho hub layout on the core.
core = g
ohl_opts = OrthoHubLayoutOptions()
ohl = OrthoHubLayout(core, ohl_opts)
ohl.layout()
# Give the trees their symmetric layout.
iel = g.getIEL()
for tree in trees:
    tree.symmetricLayout(CardinalDir_EAST, iel/4.0, iel, opts.preferConvexTrees)
# Do an ortho routing and planarise the core.
opts.routingScalar_shapeBufferDistance = 1/8.0
lor = LeaflessOrthoRouter(core, opts)
lor.route()
op = OrthoPlanariser(core)
P = op.planarise()
# Reattach trees.
reattachTrees(P, trees, opts)
# Write output.
t = P.writeTglf()
with open('./output/expand.tglf', 'w') as f:
    f.write(t)

