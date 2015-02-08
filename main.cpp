#include <QtWidgets>
#include <QtPlugin>

#include "window.h"
#include "listener.h"
#include <QString>
#include "upload_list.h"

int main(int argc, char *argv[])
{
    //AllocConsole();
    QCoreApplication::setOrganizationName("rghost");
    QCoreApplication::setOrganizationDomain("rghost.net");
    QCoreApplication::setApplicationName("uploader");
    QApplication app(argc, argv);
    for(int i = 1; i < app.arguments().size(); ++i)
        UploadList::add(new Uploading(app.arguments().at(i)));

    if (Listener::instance().create()){
        Q_INIT_RESOURCE(systray);
        Window::instance().show();
        return app.exec();
    }
    else{
        #ifdef WIN32
        DWORD message_size, bytes_written;
        HANDLE hSlot = CreateFileA(UPLOADER_MAILSLOT, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (hSlot == INVALID_HANDLE_VALUE)
            return 0;

        for(int i = 0; i < UploadList::size(); ++i) {
            message_size = (DWORD) (lstrlen((WCHAR*)UploadList::at(i)->path().utf16())+1)*sizeof(TCHAR);
            if (! WriteFile(hSlot, (ushort *)UploadList::at(i)->path().utf16(), message_size, &bytes_written, NULL)) {
                return 0;
            }
        }
        CloseHandle(hSlot);
        #endif
        return 0;
    }
}
