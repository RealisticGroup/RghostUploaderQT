#ifndef UPLOAD_MANAGER_H
#define UPLOAD_MANAGER_H

#include <QObject>
#include <QMutex>
#include <QtNetwork/QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include "math.h"
#include "payload.h"

#define xstr(s) str(s)
#define str(s) #s
#define VERSION xstr(GIT_VERSION)
#define USER_AGENT "rgup " xstr(GIT_VERSION)

class UploadManager : public QObject
{
Q_OBJECT

public:
    UploadManager(QObject* parent = 0);
    void prepareUpload();
    void startUpload();

    QFile * upload_file;
    Payload * payload;
    QString session = "";
    qint64 current_progress = 0;
    QString current_progress_descr = "";

public slots:
    void hostRequestFinished();
    void sendFileRequestFinished();

    void toggleUploadState();
    void updateUploadList();

    void updateUploadProgress(qint64 bytesSent, qint64 totalBytes);
signals:
    void updatedCurrentUploadState();

private:
    //Progressbar stuff
    QDateTime start_date, last_date;
    float maximum_speed, average_speed;
    qint64 bytes_sent = 0;
    QStringList suffixes;

    //HTTP stuff
    QNetworkAccessManager network_manager;
    QNetworkReply * network_reply;
    QJsonDocument json_document;

    void startUploadSequence();
    bool assignFileForUpload();
    void sendFile();
    void failCurrentUpload(QString message, bool startNext = true);
    void setDefaultHeaders(QNetworkRequest * request);
    bool parseResponse();

    void recordSpeed(qint64 new_bytes_sent);
};

#endif // UPLOAD_MANAGER_H
