#ifdef WIN32
#include <windows.h>
#endif

#include <QApplication>
#include "window.h"
#include <QString>
#include <QWindow>
#include "upload_list.h"

int main(int argc, char *argv[])
{
    //AllocConsole();
    SingleApplication app(argc, argv, true);

    QCoreApplication::setOrganizationName("RGhost");
    QCoreApplication::setOrganizationDomain("rghost.net");
    QCoreApplication::setApplicationName("RGhost Uploader");
    QCoreApplication::setApplicationVersion(VERSION);

    if( app.isSecondary() ) {
        qInfo() << "Already started, waking up the primary process";
        #ifdef WIN32
        AllowSetForegroundWindow(DWORD(app.primaryPid()));
        #endif
        app.sendMessage("\n"); // wake up even if no args given
        for(int i = 1; i < app.arguments().size(); ++i)
          app.sendMessage((app.arguments().at(i) + "\n").toUtf8());
        return 0;
    } else {
        Q_INIT_RESOURCE(systray);
        Window::instance().show();
        QObject::connect(
            &app,
            &SingleApplication::receivedMessage,
            &Window::instance(),
            &Window::receivedMessage
        );
    }
    
    return app.exec();
}
