#-------------------------------------------------
#
# Project created by QtCreator 2015-06-05T10:23:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TiffHDR
TEMPLATE = app

LIBS += -ltiff

SOURCES += main.cpp\
        mainwindow.cpp \
    display.cpp

HEADERS  += mainwindow.h \
    display.h

FORMS    += mainwindow.ui
