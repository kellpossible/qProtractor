#-------------------------------------------------
#
# Project created by QtCreator 2015-10-09T23:29:58
#
#-------------------------------------------------

QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qprotractor
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    protractor.cpp

HEADERS  += widget.h \
    protractor.h \
    ui_about.h

FORMS += \
    about.ui

DISTFILES += \
    logo_icon.png \
    logo_small.png
