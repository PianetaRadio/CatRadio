QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogconfig.cpp \
    dialogsetup.cpp \
    guidata.cpp \
    main.cpp \
    mainwindow.cpp \
    rigcommand.cpp \
    rigdaemon.cpp \
    rigdata.cpp \
    smeter.cpp \
    submeter.cpp \
    vfodisplay.cpp

HEADERS += \
    dialogconfig.h \
    dialogsetup.h \
    guidata.h \
    mainwindow.h \
    rigcommand.h \
    rigdaemon.h \
    rigdata.h \
    smeter.h \
    submeter.h \
    vfodisplay.h

FORMS += \
    dialogconfig.ui \
    dialogsetup.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -L$$PWD/hamlib/ -lhamlib
INCLUDEPATH += $$PWD/hamlib

VERSION = 1.0.0

RC_ICONS = catradio.ico

QMAKE_LFLAGS += -no-pie
