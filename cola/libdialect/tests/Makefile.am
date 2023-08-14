AM_CPPFLAGS = -I$(top_srcdir)

LDADD = \
  $(top_builddir)/libvpsc/libvpsc.la \
  $(top_builddir)/libcola/libcola.la \
  $(top_builddir)/libavoid/libavoid.la \
  $(top_builddir)/libdialect/libdialect.la

# Basic unit testing:
check_PROGRAMS = \
  aca assignments bbox bendcosts chainconfig01 chainconfig02 chainconfig03 \
  chainsandcycles cmplayout01 collateralexpand01 collateralexpand02 conncomps \
  containedsegment01 destress destress02 destress_aca \
  expand01 expand02 expand03 expand04 expand05 expand06 expand07 expand08 expand09 \
  extrabdrygap faceset01 faceset02 hola10 hola11 hola12 \
  hola_arpa hola_belnet hola_cernet hola_claranet hola_garr hola_janetlense hola_slovakia \
  holalonenode hola_tree inserttrees01 leaflessroute01 leaflessroute02 lookupqas nbroctal \
  nearalign01 nearalign02 nearby negativesepco negativezero nodeconfig01 nudgeopt \
  partition01 peel planarise01 planarise02 projseq01 readconstraints \
  rotate01 rotate02 rotate03 rotate04 routing01 sep_matrix_iter solidify symmtree \
  tglf01 treeboxes01 treeplacement01 treeplacement02 treeplacement03 trees trees2 vpsc01

# Test HOLA on some larger and more interesting SBGN and metro map diagrams:
# Takes a few minutes.
#check_PROGRAMS = holasbgn01 holasbgn02 holametro01 holasbgn03 holasbgn04

# HOLA speed testing on random graphs of a range of sizes and densities.
# Takes a few minutes.
# Writes timings to stdout. (But to see that you must run it outside the testing
# framework.)
#check_PROGRAMS = holaRand

aca_SOURCES = aca.cpp
assignments_SOURCES = assignments.cpp
bbox_SOURCES = bbox.cpp
bendcosts_SOURCES = bendcosts.cpp
chainconfig01_SOURCES = chainconfig01.cpp
chainconfig02_SOURCES = chainconfig02.cpp
chainconfig03_SOURCES = chainconfig03.cpp
chainsandcycles_SOURCES = chainsandcycles.cpp
cmplayout01_SOURCES = cmplayout01.cpp
collateralexpand01_SOURCES = collateralexpand01.cpp
collateralexpand02_SOURCES = collateralexpand02.cpp
conncomps_SOURCES = conncomps.cpp
containedsegment01_SOURCES = containedsegment01.cpp
destress_SOURCES = destress.cpp
destress02_SOURCES = destress02.cpp
destress_aca_SOURCES = destress_aca.cpp
expand01_SOURCES = expand01.cpp
expand02_SOURCES = expand02.cpp
expand03_SOURCES = expand03.cpp
expand04_SOURCES = expand04.cpp
expand05_SOURCES = expand05.cpp
expand06_SOURCES = expand06.cpp
expand07_SOURCES = expand07.cpp
expand08_SOURCES = expand08.cpp
expand09_SOURCES = expand09.cpp
extrabdrygap_SOURCES = extrabdrygap.cpp
faceset01_SOURCES = faceset01.cpp
faceset02_SOURCES = faceset02.cpp
hola10_SOURCES = hola10.cpp
hola11_SOURCES = hola11.cpp
hola12_SOURCES = hola12.cpp
hola_arpa_SOURCES = hola_arpa.cpp
hola_belnet_SOURCES = hola_belnet.cpp
hola_cernet_SOURCES = hola_cernet.cpp
hola_claranet_SOURCES = hola_claranet.cpp
hola_garr_SOURCES = hola_garr.cpp
hola_janetlense_SOURCES = hola_janetlense.cpp
hola_slovakia_SOURCES = hola_slovakia.cpp
holalonenode_SOURCES = holalonenode.cpp
hola_tree_SOURCES = hola_tree.cpp
inserttrees01_SOURCES = inserttrees01.cpp
leaflessroute01_SOURCES = leaflessroute01.cpp
leaflessroute02_SOURCES = leaflessroute02.cpp
lookupqas_SOURCES = lookupqas.cpp
nbroctal_SOURCES = nbroctal.cpp
nearalign01_SOURCES = nearalign01.cpp
nearalign02_SOURCES = nearalign02.cpp
nearby_SOURCES = nearby.cpp
negativesepco_SOURCES = negativesepco.cpp
negativezero_SOURCES = negativezero.cpp
nodeconfig01_SOURCES = nodeconfig01.cpp
nudgeopt_SOURCES = nudgeopt.cpp
partition01_SOURCES = partition01.cpp
peel_SOURCES = peel.cpp
planarise01_SOURCES = planarise01.cpp
planarise02_SOURCES = planarise02.cpp
projseq01_SOURCES = projseq01.cpp
readconstraints_SOURCES = readconstraints.cpp
rotate01_SOURCES = rotate01.cpp
rotate02_SOURCES = rotate02.cpp
rotate03_SOURCES = rotate03.cpp
rotate04_SOURCES = rotate04.cpp
routing01_SOURCES = routing01.cpp
sep_matrix_iter_SOURCES = sep_matrix_iter.cpp
solidify_SOURCES = solidify.cpp
symmtree_SOURCES = symmtree.cpp
tglf01_SOURCES = tglf01.cpp
treeboxes01_SOURCES = treeboxes01.cpp
treeplacement01_SOURCES = treeplacement01.cpp
treeplacement02_SOURCES = treeplacement02.cpp
treeplacement03_SOURCES = treeplacement03.cpp
trees_SOURCES = trees.cpp
trees2_SOURCES = trees2.cpp
vpsc01_SOURCES = vpsc01.cpp
holasbgn01_SOURCES = holasbgn01.cpp
holasbgn02_SOURCES = holasbgn02.cpp
holametro01_SOURCES = holametro01.cpp
holasbgn03_SOURCES = holasbgn03.cpp
holasbgn04_SOURCES = holasbgn04.cpp
holaRand_SOURCES = holaRand.cpp

TESTS = $(check_PROGRAMS)
