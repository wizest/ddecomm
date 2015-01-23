#-------------------------------------------------
#
# Project created by QtCreator 2014-10-12T22:43:59
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = DDEComm
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += c++11

TEMPLATE = app

SOURCES += main.cpp \
    ddecomm.cpp

HEADERS += \
    callback.h \
    ddecomm.h

LIBS += -lUSER32
