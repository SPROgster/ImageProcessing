#-------------------------------------------------
#
# Project created by QtCreator 2014-04-12T19:52:20
#
#-------------------------------------------------

QT       += core gui testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageProcessing
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imageentry.cpp \
    structuralElements.cpp \
    watershed.cpp

HEADERS  += mainwindow.h \
    imageentry.h \
    structuralElements.h \
    watershed.h

FORMS    += mainwindow.ui

CONFIG   += static

OTHER_FILES +=

RESOURCES += \
    ico.qrc
