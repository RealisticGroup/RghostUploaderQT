#pragma once
#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#endif
#include <QThread>

#define UPLOADER_MAILSLOT _T("\\\\.\\mailslot\\rghost_uploader_mailslot")

class Listener : public QThread {
Q_OBJECT

public:
    static Listener & instance();
    bool create();
signals:
    void UploadingsReceived();
protected:
    virtual void run();
    void read();
private:
    Listener();
    ~Listener();
#ifdef WIN32
    HANDLE hMailslot;
#endif
};
