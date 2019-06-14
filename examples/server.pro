TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    server.c \
    error.c


LIBS += ../../obj/libst.a
