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

TEMPLATE = app

SOURCES += main.cpp \
    logger.cpp \
    ddecomm.cpp

HEADERS += \
    logger.h \
    ddecomm.h

LIBS += -lUSER32
