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
    display.cpp \
    data.cpp \
    loaddialog.cpp \
    loadingdialog.cpp \
    reducer.cpp

HEADERS  += mainwindow.h \
    display.h \
    data.h \
    loaddialog.h \
    loadingdialog.h \
    reducer.h

FORMS    += mainwindow.ui \
    loaddialog.ui \
    loadingdialog.ui
