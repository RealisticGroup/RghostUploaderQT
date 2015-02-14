#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QtGui/QTabWidget>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QtNetwork/QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDateTime>
#include <QSettings>
#include "math.h"
#include "payload.h"
#include <QMutex>
#include <QtScript>
#include <QScriptEngine>
#include <QClipboard>

#define USER_AGENT "rgup 1.3"

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
    QScriptEngine script_engine;
    QScriptValue  script_value;

    QFile * upload_file;
    Payload * payload;
    QString session;

    //Progressbar stuff
    QDateTime start_date, last_date;
    float maximum_speed, average_speed;
    qint64 bytes_sent;
    QStringList suffixes;

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
    void toggleApiKey(int state);
    void updateApiKey();
    void toggleSystemTray(int state);

    void uploadRequestFinished();
    void jsonRequestFinished();
    void updateDataSendProgress(qint64 bytesSend, qint64 totalBytes);
    void uploadError(QNetworkReply::NetworkError state);

private:
    Ui::TabWidget *m_ui;

};

#endif // TABWIDGET_H
