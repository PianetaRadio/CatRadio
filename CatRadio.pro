QT += core gui widgets network serialport multimedia

CONFIG += c++17
CONFIG += sdk_no_version_check   # silences SDK 15 vs 26 warning

SOURCES += \
    mainwindow.cpp \
    guidata.cpp \
    rigcommand.cpp \
    rigdata.cpp \
    rigdaemon.cpp \
    vfodisplay.cpp \
    smeter.cpp \
    submeter.cpp \
    debugLogger.cpp \
    DialogCWKeyer.cpp \
    DialogNetRigctl.cpp \
    DialogVoiceKeyer.cpp \
    dialogsetup.cpp \
    dialogconfig.cpp \
    dialogcommand.cpp \
    dialogradioinfo.cpp \
    winkeyer.cpp \
    main.cpp

HEADERS += \
    mainwindow.h \
    guidata.h \
    rigcommand.h \
    rigdata.h \
    rigdaemon.h \
    vfodisplay.h \
    smeter.h \
    submeter.h \
    debugLogger.h \
    DialogCWKeyer.h \
    DialogNetRigctl.h \
    DialogVoiceKeyer.h \
    dialogsetup.h \
    dialogconfig.h \
    dialogcommand.h \
    dialogradioinfo.h \
    winkeyer.h

FORMS += \
    dialogcommand.ui \
    dialogconfig.ui \
    dialognetrigctl.ui \
    dialogradioinfo.ui \
    dialogsetup.ui \
    dialogvoicekeyer.ui \
    mainwindow.ui \
    DialogCWKeyer.ui \
    DialogNetRigctl.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -L/opt/homebrew/lib -L$$PWD/hamlib/ -lhamlib
INCLUDEPATH += -L/opt/homebrew/include $$PWD/hamlib

QMAKE_LFLAGS += -Wl,-rpath,\\$\$ORIGIN/hamlib/ #Set runtime shared libraries path to use local hamlib library

RESOURCES += qdarkstyle/dark/darkstyle.qrc  #Include darkstyle

VERSION = 1.5.0

RC_ICONS = catradio.ico

QMAKE_LFLAGS += -no-pie #No Position Indipendent Executable
