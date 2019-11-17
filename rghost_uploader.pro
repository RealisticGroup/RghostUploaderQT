HEADERS += window.h \
    tabwidget.h \
    upload_list.h \
    uploading.h \
    payload.h

SOURCES += window.cpp \
    tabwidget.cpp \
    main.cpp \
    upload_list.cpp \
    uploading.cpp \
    payload.cpp

RESOURCES += systray.qrc

QT += network \
    widgets

CONFIG += qt \
    c++11 \
    warn_on \
    static \
    debug_and_release

QMAKE_LFLAGS += -static-libgcc
QMAKE_CFLAGS += -std=c++11

FORMS += tabwidget.ui

RC_ICONS = images/ghost.ico
QMAKE_TARGET_COMPANY="RGhost"
QMAKE_TARGET_PRODUCT="RGhost Uploader"

GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --long --tags)
DEFINES += GIT_VERSION=$$GIT_VERSION
VERSION = $$GIT_VERSION
VERSION ~= s/-\d+-g[a-f0-9]{6,}//

CONFIG(release, debug|release) {
  win32 : QMAKE_POST_LINK = makensis rghost_uploader.nsi
}

win32 {
  # https://github.com/itay-grudev/SingleApplication/blob/master/Windows.md
  LIBS += -luser32
}

include(singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication
