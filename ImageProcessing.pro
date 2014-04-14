#-------------------------------------------------
#
# Project created by QtCreator 2014-04-12T19:52:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   += -static -static-libgcc

TARGET = ImageProcessing
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imageentry.cpp \
    structuralElements.cpp \
    gammadialog.cpp

HEADERS  += mainwindow.h \
    imageentry.h \
    structuralElements.h \
    gammadialog.h

FORMS    += mainwindow.ui \
    gammadialog.ui

OTHER_FILES += \
    res/maskButton.bmp

RESOURCES +=
