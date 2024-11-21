QT       += core gui
QT       += serialport
QT       += multimedia


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogcommand.cpp \
    dialogconfig.cpp \
    dialognetrigctl.cpp \
    dialogradioinfo.cpp \
    dialogsetup.cpp \
    dialogvoicekeyer.cpp \
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
    dialogcommand.h \
    dialogconfig.h \
    dialognetrigctl.h \
    dialogradioinfo.h \
    dialogsetup.h \
    dialogvoicekeyer.h \
    guidata.h \
    mainwindow.h \
    rigcommand.h \
    rigdaemon.h \
    rigdata.h \
    smeter.h \
    submeter.h \
    vfodisplay.h

FORMS += \
    dialogcommand.ui \
    dialogconfig.ui \
    dialognetrigctl.ui \
    dialogradioinfo.ui \
    dialogsetup.ui \
    dialogvoicekeyer.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -L$$PWD/hamlib/ -lhamlib
INCLUDEPATH += $$PWD/hamlib

RESOURCES += qdarkstyle/dark/darkstyle.qrc

VERSION = 1.5.0

RC_ICONS = catradio.ico

QMAKE_LFLAGS += -no-pie
