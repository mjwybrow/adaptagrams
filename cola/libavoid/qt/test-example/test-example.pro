
TEMPLATE = app
DEPENDPATH =  ../../..
INCLUDEPATH = ../../..
TARGET = example

DESTDIR = ..

CONFIG -= qt

LIBS += -L$$DESTDIR -lavoid

SOURCES += \ 
	../../../libavoid/tests/example.cpp

