#include "upload_manager.h"
#include "upload_list.h"
#include "window.h"

UploadManager::UploadManager(QObject * parent) : QObject(parent), network_manager(), json_document()
{
    suffixes << "B" << "KB" << "MB" << "GB" << "TB" << "PB";

    connect(UploadList::instance(), SIGNAL(changedList()), this, SLOT(updateUploadList()));
}

void UploadManager::toggleUploadState() {
    if (UploadList::instance()->current_uploading == NULL)
        startUploadSequence();
    else {
        network_reply->abort();
        network_reply->deleteLater();
        UploadList::instance()->fail_all_pending(tr("Aborted"));
        failCurrentUpload(tr("Aborted"), false);
    }
}

void UploadManager::updateUploadList() {
    if (UploadList::instance()->current_uploading == NULL)
        startUploadSequence();
}


void UploadManager::startUploadSequence() {
    if (!assignFileForUpload())
        return

    UploadList::instance()->change_current_state(tr("uploading"), tr("Preparing..."));
    current_progress = 0;
    emit updatedCurrentUploadState();

    QNetworkRequest request;
    request.setUrl(QUrl("https://rghost.net/api/upload_url"));
    setDefaultHeaders(&request);
    network_reply = network_manager.get(request);
    connect(network_reply, SIGNAL(finished()), this, SLOT(hostRequestFinished()));
}

bool UploadManager::assignFileForUpload() {
    if (UploadList::instance()->new_current_uploading() == NULL)
        return false;
    if (!QFile::exists(UploadList::instance()->current_uploading->path())) {
        failCurrentUpload(tr("Error: file does not exist"));
        return false;
    }
    upload_file = new QFile(UploadList::instance()->current_uploading->path());
    if ( !upload_file->open(QIODevice::ReadOnly) ) {
        failCurrentUpload(tr("Unable to open the file %1: %2.").arg(UploadList::instance()->current_uploading->path()).arg(upload_file->errorString()));
        return false;
    }
    return true;
}

void UploadManager::setDefaultHeaders(QNetworkRequest * request) {
    request->setRawHeader("Accept-Language","en");
    request->setRawHeader("Accept","application/json");
    request->setRawHeader("User-Agent", USER_AGENT);
    request->setRawHeader("Cookie", session.toUtf8());
    if ( Window::isApiKeyEntered())
        request->setRawHeader("X-API-Key", Window::instance().settings.value("api_key").toString().toUtf8());
}

void UploadManager::hostRequestFinished() {
    if (!parseResponse())
        return;

    qint64 upload_limit = json_document["upload_limit"].toInt();
    if ( upload_file->size() > upload_limit*1024*1024) {
        upload_file->close();
        upload_file = NULL;
        failCurrentUpload(tr("Upload size limit (%1 MB)").arg(upload_limit));
        return;
    }

    sendFile();
}

bool UploadManager::parseResponse() {
    session = network_reply->rawHeader("Set-Cookie");
    qDebug() << "received session:\n" << session;

    QByteArray reply = network_reply->readAll();
    json_document = QJsonDocument::fromJson(reply);

    qDebug() << "received response:\n" << reply << "\n" << json_document;

    QString message = json_document["error"].toString();
    if (message == "" && network_reply->error() != QNetworkReply::NoError)
        message = network_reply->errorString();

    network_reply->deleteLater();

    if (message != "") {
        failCurrentUpload(tr("Upload failed: %1.").arg(message));
        return false;
    } else
        return true;
}

void UploadManager::sendFile() {
    QNetworkRequest request;
    UploadList::instance()->change_current_state(tr("uploading"), tr("Uploading..."));

    request.setUrl(QUrl(json_document["upload_url"].toString()));
    setDefaultHeaders(&request);
    request.setRawHeader("Content-type", QString("multipart/form-data; boundary=" + Payload::boundary).toUtf8());
    request.setAttribute(QNetworkRequest::DoNotBufferUploadDataAttribute, true);

    payload = new Payload(upload_file, json_document);
    start_date = last_date = QDateTime::currentDateTime();
    network_reply = network_manager.post(request, payload);

    connect(network_reply, SIGNAL(finished()), this, SLOT(sendFileRequestFinished()));
    connect(network_reply, SIGNAL(uploadProgress(qint64, qint64) ), this, SLOT(updateUploadProgress(qint64,qint64)));
}

void UploadManager::sendFileRequestFinished() {
    if (!parseResponse())
       return;
    QString message = json_document["url"].toString();
    if (message == "")
        message = "Something went wrong, please contact support@rghost.net";
    UploadList::instance()->change_current_state(tr("uploaded"), message);
    current_progress = 1000;
    emit updatedCurrentUploadState();
    payload->deleteLater();
    network_reply->deleteLater();
    upload_file = NULL;
    payload = NULL;
    UploadList::instance()->stop_current_uploading();
    UploadManager::startUploadSequence();
}

void UploadManager::failCurrentUpload(QString description, bool startNext) {
    UploadList::instance()->change_current_state(tr("error"), description);
    UploadList::instance()->stop_current_uploading();
    if (startNext)
      UploadManager::startUploadSequence();
}

void UploadManager::updateUploadProgress(qint64 bytesSent, qint64 totalBytes) {
    if (bytesSent <= 0)
        return;

    qDebug() << "progress event: " << bytesSent << "/" << totalBytes;
    recordSpeed(bytesSent);
    current_progress = round(1000 * (float(bytesSent) / float(totalBytes)));

    qint64 interval = start_date.secsTo(last_date);

    if (interval == 0) {
        emit updatedCurrentUploadState();
        return;
    }
    float average_speed = float(bytesSent) / float(interval);
    qint64 eta_seconds = round(float(totalBytes - bytesSent) / average_speed);

    QString size_str, speed_str, time_str;

    qint64 size = bytesSent, max_size = totalBytes;

    int i = 0;

    for(; i < suffixes.size(); ++i) {
        if(max_size < 2048)
            break;
        size /= 1024;
        max_size /= 1024;
    }

    int hours = round(eta_seconds / 3600);
    int temp = round(eta_seconds % 3600);
    int min = round(temp / 60);
    int sec = round(temp % 60);

    size_str = tr("%1 / %2 %3  ").arg(QString::number(size, 'g', 2)).arg(QString::number(max_size, 'g', 2)).arg(suffixes[i]);
    speed_str = tr("%1 KBps  ").arg(round(average_speed/1024));
    if (hours > 0)
        time_str = tr("%1h %2m").arg(hours).arg(min);
    else if (min > 5)
        time_str = tr("%1 m").arg(min);
    else
        time_str = tr("%1m %2s").arg(min).arg(sec);

    current_progress_descr = size_str + speed_str + time_str;
    emit updatedCurrentUploadState();
    qDebug() << "recording progress:\n"<< current_progress << "\n" << current_progress_descr;
}



void UploadManager::recordSpeed(qint64 new_bytes_sent) {
    QDateTime current_time = QDateTime::currentDateTime();
    if(last_date != start_date) {
        int interval = last_date.secsTo(current_time);
        if(interval > 0) {
            float avg = round(float(new_bytes_sent - bytes_sent)/interval);
            if(maximum_speed < avg)
                maximum_speed = avg;
        }
    }
    last_date = current_time;
    bytes_sent = new_bytes_sent;
}
