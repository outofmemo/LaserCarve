#-------------------------------------------------
#
# Project created by QtCreator 2018-02-04T20:01:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LaserCarve
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    machine_base.cpp \
    basicmode.cpp \
    bitmapmode.cpp \
    scanmode.cpp \
    continuationmode.cpp \
    ncmode.cpp \
    logutil.cpp \
    logtext.cpp \
    homewidget.cpp \
    bitmapwindow.cpp \
    ncwindow.cpp

HEADERS  += mainwindow.hh \
    machine_base.hh \
    basicmode.hh \
    bitmapmode.hh \
    scanmode.hh \
    continuationmode.hh \
    ncmode.hh \
    logutil.hh \
    logtext.hh \
    homewidget.hh \
    bitmapwindow.hh \
    ncwindow.hh

FORMS    += mainwindow.ui

target.path = /root/workspace
INSTALLS += target

RESOURCES += \
    icon.qrc
