#pragma once
#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QFile>
#include <QDateTime>
#include <QSettings>
#include <QWidget>
#include <QMutex>
#include <QClipboard>
#include "upload_manager.h"
#include "ui_tabwidget.h"
#include "upload_list.h"
#include "uploading.h"

namespace Ui {
    class TabWidget;
}

class TabWidget : public QTabWidget {
    Q_OBJECT
public:
    TabWidget(QWidget *parent = 0);
    ~TabWidget();

    QStandardItemModel * uploadings_model;
    UploadManager * upload_manager;

    // Helpers
    void debugLog(QString message);

protected:
    void changeEvent(QEvent *e);

public slots:
    void updateUploadState();
    void updateUploadList();

    void uploadDialog();
    void clearTableView();
    void copyTableView();
    void updateApiKey();
    void toggleSystemTray(int state);

private:
    Ui::TabWidget *m_ui;
    void loadOptions();

    void updateTableView();
    void updateButtonsState();
};

#endif // TABWIDGET_H
