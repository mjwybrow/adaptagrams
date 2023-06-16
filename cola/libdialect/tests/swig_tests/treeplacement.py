from pkg.adaptagrams import *

opts = HolaOpts()
# Build a graph
g = buildGraphFromTglfFile("../graphs/special/treeplacement3.tglf")
# Peel.
trees = peel(g)
# Give the trees their symmetric layout.
iel = g.getIEL()
for tree in trees:
    tree.symmetricLayout(CardinalDir_EAST, iel/4.0, iel, opts.preferConvexTrees)
# Do an ortho routing and planarise the core.
opts.routingScalar_shapeBufferDistance = 1/8.0
lor = LeaflessOrthoRouter(g, opts)
lor.route()
op = OrthoPlanariser(g);
P = op.planarise();
# Compute the face set.
faceSet = FaceSet(P)
# Get the tree rooted at node 0.
for tree in trees:
    if tree.getRootNodeID() == 0:
        t0 = tree
        break
# Get all possible tree placements.
tps = faceSet.listAllPossibleTreePlacements(t0)
# Choose best placement; should be NORTH.
best1 = chooseBestPlacement(tps, opts)
assert best1.getPlacementDir() == CompassDir_NORTH
# But if we do not favour external placement, then best should be
# SOUTH, dur to isolation from the tree rooted at node 1.
opts.treePlacement_favourExternal = False
best2 = chooseBestPlacement(tps, opts)
assert best2.getPlacementDir() == CompassDir_SOUTH

