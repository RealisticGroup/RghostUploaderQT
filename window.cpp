#include <QtWidgets>
#include "window.h"
#include "tabwidget.h"
#include "upload_list.h"
Window * Window::shared_instance = NULL;

Window & Window::instance() {
    if (shared_instance == NULL)
        new Window;
    return *shared_instance;
}
Window::Window() : settings() {
    shared_instance = this;
    createActions();
    createTrayIcon();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    tabWidget = new TabWidget();
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    setWindowTitle(tr("RGhost Uploader"));
    // TODO: make configurable
    // setWindowFlags(Qt::WindowStaysOnTopHint);
    setAcceptDrops(true);
}

void Window::setVisible(bool visible) {
    minimizeAction->setEnabled(visible);
    restoreAction->setEnabled(!visible);
    QDialog::setVisible(visible);
}

void Window::closeEvent(QCloseEvent *event) {
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

void Window::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void Window::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        for(int i = 0; i < event->mimeData()->urls().size(); ++i)
            UploadList::instance()->add(event->mimeData()->urls().at(i).toLocalFile());
        UploadingsReceived();
    }
    event->acceptProposedAction();
}

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        setVisible(!isVisible());
        break;
    case QSystemTrayIcon::MiddleClick:
        break;
    default:
        ;
    }
}


void Window::createActions() {
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void Window::createTrayIcon() {
    QIcon icon = QIcon(":/images/ghost.png");
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    trayIcon->setIcon(icon);
    setWindowIcon(icon);
}

void Window::receivedMessage(int instanceId, QByteArray message) {
    instanceStarted();
    tabWidget->debugLog("received message:");
    tabWidget->debugLog(QString(message));
    tabWidget->debugLog("adding files:");
    QStringList list = QString(message).split("\n", QString::SkipEmptyParts);

    for ( const auto& path : list  ) {
        tabWidget->debugLog(path);
        UploadList::instance()->add(path);
    }
    UploadingsReceived();
}

bool Window::isApiKeyEntered() {
  return Window::instance().settings.value("api_key").toString().size() > 10;
}

void Window::instanceStarted() {
    qInfo() << "Woken up by a message";
    setVisible(true);
    QApplication::setActiveWindow(this);
    show();
    raise();
    activateWindow();
}
