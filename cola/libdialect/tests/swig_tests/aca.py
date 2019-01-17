from pkg.adaptagrams import *

# Build a graph.
g = buildGraphFromTglfFile("../graphs/random/v30e33.tglf")
# Do ACA layout.
aca = ACALayout(g)
aca.layout()
aca.updateGraph()
# Record the layout.
with open('./output/aca.tglf', 'w') as f: f.write(g.writeTglf())
with open('./output/svg/aca.svg', 'w') as f: f.write(g.writeSvg())

