
TEMPLATE = lib
TARGET = avoid

CONFIG += shared

INCLUDEPATH += ../../..
DEPENDPATH += ../../..

CONFIG -= qt

DESTDIR = ..

# Input
SOURCES += ../../../libavoid/connector.cpp ../../../libavoid/geometry.cpp ../../../libavoid/geomtypes.cpp ../../../libavoid/graph.cpp ../../../libavoid/makepath.cpp ../../../libavoid/orthogonal.cpp ../../../libavoid/router.cpp ../../../libavoid/shape.cpp ../../../libavoid/timer.cpp ../../../libavoid/vertices.cpp ../../../libavoid/viscluster.cpp ../../../libavoid/visibility.cpp ../../../libavoid/vpsc.cpp ../../../libavoid/connend.cpp ../../../libavoid/connectionpin.cpp ../../../libavoid/junction.cpp ../../../libavoid/obstacle.cpp ../../../libavoid/mtst.cpp ../../../libavoid/hyperedge.cpp ../../../libavoid/hyperedgetree.cpp ../../../libavoid/actioninfo.cpp ../../../libavoid/scanline.cpp ../../../libavoid/hyperedgeimprover.cpp
HEADERS += ../../../libavoid/assertions.h ../../../libavoid/connector.h ../../../libavoid/debug.h ../../../libavoid/geometry.h ../../../libavoid/geomtypes.h ../../../libavoid/graph.h ../../../libavoid/libavoid.h ../../../libavoid/makepath.h ../../../libavoid/orthogonal.h ../../../libavoid/router.h ../../../libavoid/shape.h ../../../libavoid/timer.h ../../../libavoid/vertices.h ../../../libavoid/viscluster.h ../../../libavoid/visibility.h ../../../libavoid/vpsc.h ../../../libavoid/connend.h ../../../libavoid/connectionpin.h ../../../libavoid/junction.h ../../../libavoid/obstacle.h ../../../libavoid/mtst.h ../../../libavoid/hyperedge.h ../../../libavoid/hyperedgetree.h ../../../libavoid/actioninfo.h ../../../libavoid/scanline.h ../../../libavoid/dllexport.h ../../../libavoid/hyperedgeimprover.h
