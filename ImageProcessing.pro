#-------------------------------------------------
#
# Project created by QtCreator 2014-04-12T19:52:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageProcessing
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imageentry.cpp \
    structuralElements.cpp \
    gmm.cpp \
    graph.cpp \
    progressiveCut.cpp \
    maxflow.cpp

HEADERS  += mainwindow.h \
    imageentry.h \
    structuralElements.h \
    gmm.h \
    graph.h \
    progressiveCut.h \
    block.h \
    config.h

FORMS    += mainwindow.ui

CONFIG   += static

OTHER_FILES +=

RESOURCES += \
    ico.qrc
