
TEMPLATE = app
DEPENDPATH =  ../../..
INCLUDEPATH = ../../..
TARGET = example

CONFIG -= qt

# DESTDIR = $$DUNNARTBASE/build

LIBS += -L../../../libavoid/ -lavoid

SOURCES += \ 
	../../../libavoid/tests/example.cpp

