#include "listener.h"
#include "upload_list.h"

static Listener * shared_instance = &Listener::instance();

void Listener::read()
{
#ifdef WIN32
    DWORD count = 0;
    DWORD message_size;
    WaitForSingleObject(hMailslot, INFINITE);
    GetMailslotInfo(hMailslot, 0, &message_size, &count, 0);
    if (count > 0) {
        for(;count > 0;) {
            TCHAR * buf = (TCHAR*)malloc(message_size);
            DWORD read;

            ReadFile(hMailslot, buf, message_size, &read, 0);
            UploadList::add(QString::fromUtf16((ushort *)buf));
            GetMailslotInfo(hMailslot, 0, &message_size, &count, 0);

            free(buf);
        }
        UploadingsReceived();
    }
#endif
}

bool Listener::create()
{
#ifdef WIN32
    hMailslot = CreateMailslotA(UPLOADER_MAILSLOT, 0, MAILSLOT_WAIT_FOREVER, 0);
    if(hMailslot == INVALID_HANDLE_VALUE) {
        return false;
    }else{
        start();
        return true;
    }
#else
    return true;
#endif
}

void Listener::run()
{
    for(;;) {
        msleep( 500 );
        read();
    }
}


Listener & Listener::instance()
{
    if(!shared_instance)
        shared_instance = new Listener();
    return *shared_instance;
}

Listener::Listener(){}
Listener::~Listener(){}
