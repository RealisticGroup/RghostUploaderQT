#include "tabwidget.h"
#include "ui_tabwidget.h"
#include "upload_list.h"
#include "uploading.h"
#include "window.h"
#include <QDesktopServices>


TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent),
    m_ui(new Ui::TabWidget)
{
    m_ui->setupUi(this);
    m_ui->label_version->setText(VERSION);

    if (isApiKeyEntered())
        setCurrentWidget(m_ui->tab_files);
    else
        setCurrentWidget(m_ui->tab_settings);
    connect(m_ui->pushButton_upload, SIGNAL(clicked()), this, SLOT(uploadDialog()));
    connect(m_ui->pushButton_clear, SIGNAL(clicked()), this, SLOT(clearTableView()));
    connect(m_ui->pushButton_copy, SIGNAL(clicked()), this, SLOT(copyTableView()));
    uploadings_model = new QStandardItemModel(0,2);

    m_ui->tableView->setModel(uploadings_model);

    connect(m_ui->pushButton_control, SIGNAL(clicked()), this, SLOT(controlUpload()));
    suffixes << "B" << "KB" << "MB" << "GB" << "TB" << "PB";
    loadOptions();
    connect(&Window::instance(), SIGNAL(UploadingsReceived()), this, SLOT(newUpload()));
    connect(m_ui->lineEdit_api,SIGNAL(editingFinished()), this, SLOT(updateApiKey()));
    connect(m_ui->checkBox_systemtray, SIGNAL(stateChanged(int)), this, SLOT(toggleSystemTray(int)));
    newUpload();
}

TabWidget::~TabWidget() {
    delete m_ui;
}

void TabWidget::changeEvent(QEvent *e) {
    QTabWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }

}

void TabWidget::uploadDialog() {
    QFileDialog dialog(this, tr("Select files to upload"));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setDirectory(settings.value("upload_source",
                                       QStandardPaths::standardLocations(QStandardPaths::HomeLocation)).toString());

    if (dialog.exec()) {
        for (int i = 0; i < dialog.selectedFiles().size(); ++i)
            UploadList::add(dialog.selectedFiles().at(i));
        settings.setValue("upload_source", dialog.directory().path());
        newUpload();
    }
}

void TabWidget::updateTableView() {
    QList<QStandardItem *> items;
    QStandardItem *filepath, *filestatus;
    QFileInfo *fileinfo;
    QStringList horzHeaders;
    horzHeaders << "Name" << "URL";

    uploadings_model->clear();
    uploadings_model->setHorizontalHeaderLabels(horzHeaders);

    for (int row = 0; row < UploadList::size(); ++row) {
        items.clear();
        fileinfo = new QFileInfo(UploadList::at(row)->path());
        filepath = new QStandardItem(fileinfo->fileName());
        filestatus = new QStandardItem(UploadList::at(row)->descr());
        delete fileinfo;
        items.append(filepath);
        items.append(filestatus);
        uploadings_model->insertRow(row, items);
    }
    if (UploadList::size() > 0) {
        m_ui->pushButton_clear->setEnabled(true);
        m_ui->pushButton_copy->setEnabled(true);
    } else {
        m_ui->pushButton_clear->setEnabled(false);
        m_ui->pushButton_copy->setEnabled(false);
    }
    if (UploadList::current_uploading == NULL)
        m_ui->pushButton_control->setEnabled(false);
    else
        m_ui->pushButton_control->setEnabled(true);
}

void TabWidget::clearTableView() {
    UploadList::clear();
    updateTableView();
}

void TabWidget::copyTableView() {
     QClipboard *clipboard = QApplication::clipboard();
     QString clipboardText;
     for (int row = 0; row < UploadList::size(); ++row) {
         clipboardText.append(UploadList::at(row)->descr());
         clipboardText.append("\n");
     }
     if (!clipboardText.isEmpty()) {
         clipboard->setText(clipboardText);
     }
}

void TabWidget::newUpload(){
    prepareUpload();
    updateTableView();
}

void TabWidget::prepareUpload() {
    if (UploadList::new_current_uploading() == NULL)
        return;

    if (!QFile::exists(UploadList::current_uploading->path())) {
        UploadList::current_uploading->descr(tr("Error: file does not exist"));
        UploadList::current_uploading->state(tr("error"));
        UploadList::current_uploading = NULL;
        newUpload();
        return;
    }
    upload_file = new QFile(UploadList::current_uploading->path());
    if ( !upload_file->open(QIODevice::ReadOnly) ) {
        UploadList::current_uploading->descr(tr("Unable to open the file %1: %2.").arg(
                UploadList::current_uploading->path()).arg(upload_file->errorString()));
        UploadList::current_uploading->state(tr("error"));
        UploadList::current_uploading = NULL;
        newUpload();
        return;
    }

    m_ui->progressBar->setValue(0);
    UploadList::current_uploading->state(tr("uploading"));
    UploadList::current_uploading->descr(tr("Uploading..."));

    QNetworkRequest request;
    request.setUrl(QUrl("https://rghost.net/multiple/upload_host"));
    request.setRawHeader("Host", "rghost.net");
    request.setRawHeader("Accept-Language","en");
    request.setRawHeader("User-Agent", USER_AGENT);
    if (isApiKeyEntered())
        request.setRawHeader("X-API-Key", settings.value("api_key").toString().toUtf8());

    reply = network_manager.get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(uploadError()));
    connect(reply, SIGNAL(finished()), this, SLOT(jsonRequestFinished()));
}


void TabWidget::startUpload() {
    QString uri;
    m_ui->label_progress->setText(tr("Uploading..."));

    QNetworkRequest request;
    if (json_document["premium"].toBool() == true)
        uri = QString("/premium/files");
    else
        uri = QString("/files");

    request.setUrl(QUrl(QString("https://") + json_document["upload_host"].toString() + uri));
    request.setRawHeader("Host", json_document["upload_host"].toString().toUtf8());
    request.setRawHeader("Accept-Language","en");
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setRawHeader("Cookie", session.toUtf8());
    request.setRawHeader("Content-type", QString("multipart/form-data; boundary=" + Payload::boundary).toUtf8());
    if (isApiKeyEntered())
        request.setRawHeader("X-API-Key", settings.value("api_key").toString().toUtf8());

    request.setAttribute(QNetworkRequest::DoNotBufferUploadDataAttribute, true);
    payload = new Payload(upload_file, json_document);

    start_date = last_date = QDateTime::currentDateTime();
    reply = network_manager.post(request, payload);
    connect(reply, SIGNAL(finished()), this, SLOT(uploadRequestFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(uploadError()));
    connect(reply, SIGNAL(uploadProgress(qint64, qint64) ), this, SLOT(updateDataSendProgress(qint64,qint64)));

    //_cprintf("uploading...\n");
}

void TabWidget::updateDataSendProgress(qint64 bytesSent, qint64 totalBytes) {
    //_cprintf("updateDataSendProgress %d - %d\n", bytesSend, totalBytes);
    recordSpeed(bytesSent);
    qint64 interval = start_date.secsTo(last_date);
    if (interval == 0)
        return;
    float average_speed = float(bytesSent) / float(interval);
    qint64 eta_seconds = round(float(totalBytes - bytesSent) / average_speed);

    m_ui->progressBar->setMaximum(1000);
    m_ui->progressBar->setValue(round(1000 * (float(bytesSent) / float(totalBytes))));

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

    m_ui->label_progress->setText(size_str + speed_str + time_str);
}

void TabWidget::uploadRequestFinished() {
    if (reply->error() == QNetworkReply::NoError) {
        UploadList::current_uploading->descr(reply->rawHeader("Location"));
        UploadList::current_uploading->state(tr("uploaded"));
        m_ui->label_progress->setText(tr("Finished"));
        m_ui->progressBar->setValue(m_ui->progressBar->maximum());
    }
    UploadList::current_uploading = NULL;
    upload_file = NULL;
    payload->deleteLater();
    payload = NULL;
    reply->deleteLater();
    newUpload();
}

void TabWidget::jsonRequestFinished() {
    qint64 upload_limit;

    if (reply->error() == QNetworkReply::NoError) {
        json_document = QJsonDocument::fromJson(reply->readAll());
        session = reply->rawHeader("Set-Cookie");
        reply->deleteLater();
        upload_limit = json_document["upload_limit"].toInt();

        if ( upload_file->size() > upload_limit*1024*1024) {
            UploadList::current_uploading->descr(tr("Upload size limit (%1 MB)").arg(upload_limit));
            UploadList::current_uploading->state(tr("error"));
            UploadList::current_uploading = NULL;
            upload_file->close();
            upload_file = NULL;
            newUpload();
            return;
        }
        startUpload();
    }
    else
        reply->deleteLater();
}
void TabWidget::uploadError() {
    json_document = QJsonDocument::fromJson(reply->readAll());
     UploadList::current_uploading->descr(tr("Upload failed: %1 (%2).").arg(json_document["error"].toString()).arg(reply->errorString()));
    UploadList::current_uploading->state(tr("error"));
    UploadList::current_uploading = NULL;
    newUpload();
}

void TabWidget::recordSpeed(qint64 new_bytes_sent) {
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

void TabWidget::controlUpload() {
    if (UploadList::current_uploading == NULL) {
        newUpload();
    }
    else {
        reply->abort();
    }
}

void TabWidget::updateApiKey() {
    settings.setValue("api_key", m_ui->lineEdit_api->text());
}

bool TabWidget::isApiKeyEntered(){
  return settings.value("api_key").toString().size() > 10;
}

void TabWidget::loadOptions() {
    if( settings.value("system_tray_enabled").toBool()) {
        m_ui->checkBox_systemtray->setCheckState(Qt::Checked);
        toggleSystemTray(Qt::Checked);
    }
    m_ui->lineEdit_api->setText(settings.value("api_key").toString());
}

void TabWidget::toggleSystemTray(int state) {
    bool tray_enabled = false;
    if (state == Qt::Checked)
        tray_enabled = true;
    QApplication::setQuitOnLastWindowClosed(!tray_enabled);
    Window::instance().trayIcon->setVisible(tray_enabled);
    settings.setValue("system_tray_enabled", tray_enabled);
}
