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
    kis_cubic_curve.cpp \
    kis_curve_widget.cpp

HEADERS  += mainwindow.h \
    imageentry.h \
    structuralElements.h \
    kis_cubic_curve.h \
    kis_curve_widget.h \
    kis_curve_widget_p.h \

FORMS    += mainwindow.ui

CONFIG   += static

OTHER_FILES += \
    res/maskButton.bmp

RESOURCES +=
