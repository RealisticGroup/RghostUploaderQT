HEADERS = window.h \
    tabwidget.h \
    upload_list.h \
    uploading.h \
    listener.h \
    payload.h
SOURCES = window.cpp \
    tabwidget.cpp \
    main.cpp \
    upload_list.cpp \
    uploading.cpp \
    listener.cpp \
    payload.cpp
RESOURCES = systray.qrc
QT += network \
    widgets \
    script \
    svg
CONFIG += qt \
    warn_on \
    static
QMAKE_LFLAGS += -static-libgcc
FORMS += tabwidget.ui
RC_FILE = rghost_uploader.rc

CONFIG(release, debug|release) {
  win32 : QMAKE_POST_LINK = upx --best release/$(TARGET)
}
