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
    gui/mainwindow.cpp \
    gui/display.cpp \
    rendering/data.cpp \
    gui/loaddialog.cpp \
    util/loadingdialog.cpp \
    rendering/reducer.cpp \
    gui/optionsdialog.cpp

HEADERS  += gui/mainwindow.h \
    gui/display.h \
    rendering/data.h \
    gui/loaddialog.h \
    util/loadingdialog.h \
    rendering/reducer.h \
    gui/optionsdialog.h

FORMS    += gui/mainwindow.ui \
    gui/loaddialog.ui \
    util/loadingdialog.ui \
    gui/optionsdialog.ui
