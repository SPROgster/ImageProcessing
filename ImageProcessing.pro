#-------------------------------------------------
#
# Project created by QtCreator 2014-04-12T19:52:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageProcessing
TEMPLATE = app
CONFIG += static


SOURCES += main.cpp\
    mainwindow.cpp \
    imageentry.cpp \
    structuralElements.cpp \
    morphology.cpp \
    coindialog.cpp \
    coinviewdialog.cpp


HEADERS  += mainwindow.h \
    imageentry.h \
    structuralElements.h \
    morphology.h \
    coindialog.h \
    coinviewdialog.h


FORMS    += mainwindow.ui \
    coindialog.ui \
    coinviewdialog.ui

OTHER_FILES +=

RESOURCES += \
    ico.qrc
