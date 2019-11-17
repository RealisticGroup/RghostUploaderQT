#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QtNetwork/QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDateTime>
#include <QSettings>
#include <QWidget>
#include "math.h"
#include "payload.h"
#include <QMutex>
#include <QClipboard>
#include <QJsonDocument>

#define xstr(s) str(s)
#define str(s) #s
#define VERSION xstr(GIT_VERSION)
#define USER_AGENT "rgup " xstr(GIT_VERSION)

namespace Ui {
    class TabWidget;
}

class TabWidget : public QTabWidget {
    Q_OBJECT
public:
    TabWidget(QWidget *parent = 0);
    ~TabWidget();

    QSettings settings;
    QStandardItemModel *uploadings_model;
    QMutex upload_mutex;

    //HTTP stuff
    QNetworkAccessManager network_manager;
    QNetworkReply * reply;
    QJsonDocument  json_document;

    QFile * upload_file;
    Payload * payload;
    QString session;

    //Progressbar stuff
    QDateTime start_date, last_date;
    float maximum_speed, average_speed;
    qint64 bytes_sent;
    QStringList suffixes;

    // Helpers
    bool isApiKeyEntered();
    void recordSpeed(qint64 new_bytes_sent);
    void loadOptions();

protected:
    void changeEvent(QEvent *e);

public slots:
    void uploadDialog();
    void updateTableView();
    void clearTableView();
    void copyTableView();
    void newUpload();
    void prepareUpload();
    void startUpload();
    void controlUpload();
    void updateApiKey();
    void toggleSystemTray(int state);

    void uploadRequestFinished();
    void jsonRequestFinished();
    void updateDataSendProgress(qint64 bytesSend, qint64 totalBytes);
    void uploadError();

private:
    Ui::TabWidget *m_ui;

};

#endif // TABWIDGET_H
