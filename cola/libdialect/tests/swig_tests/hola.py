from pkg.adaptagrams import *

# Build a graph.
g = buildGraphFromTglfFile("../graphs/random/v30e33.tglf")
# Do HOLA layout.
opts = HolaOpts()
doHOLA(g, opts)
# Record the layout.
with open('./output/hola.tglf', 'w') as f: f.write(g.writeTglf())
with open('./output/svg/hola.svg', 'w') as f: f.write(g.writeSvg())

