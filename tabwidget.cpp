#include "tabwidget.h"
#include <QDesktopServices>
#include <QProcessEnvironment>
#include "window.h"

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent),
    m_ui(new Ui::TabWidget)
{
    m_ui->setupUi(this);
    loadOptions();

    if (Window::isApiKeyEntered())
        setCurrentWidget(m_ui->tab_files);
    else
        setCurrentWidget(m_ui->tab_settings);

    if (QProcessEnvironment::systemEnvironment().value("RGHOST_DEBUG", "false") == "false")
        removeTab(indexOf(m_ui->tab_logs));

    uploadings_model = new QStandardItemModel(0,2);
    m_ui->tableView->setModel(uploadings_model);

    m_ui->progressBar->setMaximum(1000);

    connect(m_ui->pushButton_upload, SIGNAL(clicked()), this, SLOT(uploadDialog()));
    connect(m_ui->pushButton_clear, SIGNAL(clicked()), this, SLOT(clearTableView()));
    connect(m_ui->pushButton_copy, SIGNAL(clicked()), this, SLOT(copyTableView()));

    connect(m_ui->lineEdit_api,SIGNAL(editingFinished()), this, SLOT(updateApiKey()));
    connect(m_ui->checkBox_systemtray, SIGNAL(stateChanged(int)), this, SLOT(toggleSystemTray(int)));

    connect(&Window::instance(), SIGNAL(UploadingsReceived()), this, SLOT(updateUploadList()));

    connect(UploadList::instance(), SIGNAL(changedList()), this, SLOT(updateUploadList()));
    connect(UploadList::instance(), SIGNAL(changedCurrent()), this, SLOT(updateUploadList()));

    upload_manager = new UploadManager(0);

    connect(m_ui->pushButton_control, SIGNAL(clicked()), upload_manager, SLOT(toggleUploadState()));
    connect(upload_manager, SIGNAL(updatedCurrentUploadState()), this, SLOT(updateUploadState()));
    updateUploadList();
}

void TabWidget::updateUploadList() {
    updateTableView();
    updateButtonsState();
}

void TabWidget::updateUploadState(){
    m_ui->progressBar->setValue(upload_manager->current_progress);
    m_ui->label_progress->setText(upload_manager->current_progress_descr);

    if (upload_manager->current_progress == 0)
        m_ui->label_progress->setText(tr("Uploading..."));

    updateUploadList();
}

void TabWidget::uploadDialog() {
    QFileDialog dialog(this, tr("Select files to upload"));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setDirectory(Window::instance().settings.value("upload_source", QStandardPaths::standardLocations(QStandardPaths::HomeLocation)).toString());

    if (dialog.exec()) {
        for (auto &selected_file : dialog.selectedFiles())
            UploadList::instance()->add(selected_file);
        Window::instance().settings.setValue("upload_source", dialog.directory().path());
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

    for (int row = 0; row < UploadList::instance()->size(); ++row) {
        items.clear();
        fileinfo = new QFileInfo(UploadList::instance()->at(row)->path());
        filepath = new QStandardItem(fileinfo->fileName());
        filestatus = new QStandardItem(UploadList::instance()->at(row)->descr());
        delete fileinfo;
        items.append(filepath);
        items.append(filestatus);
        uploadings_model->insertRow(row, items);
    }
}

void TabWidget::updateButtonsState() {
    if (UploadList::instance()->size() > 0) {
        m_ui->pushButton_clear->setEnabled(true);
        m_ui->pushButton_copy->setEnabled(true);
    } else {
        m_ui->pushButton_clear->setEnabled(false);
        m_ui->pushButton_copy->setEnabled(false);
    }
    if (UploadList::instance()->current_uploading == NULL)
        m_ui->pushButton_control->setEnabled(false);
    else
        m_ui->pushButton_control->setEnabled(true);
}

void TabWidget::clearTableView() {
    UploadList::instance()->clear();
}

void TabWidget::copyTableView() {
     QClipboard *clipboard = QApplication::clipboard();
     QString clipboardText;
     for (int row = 0; row < UploadList::instance()->size(); ++row) {
         clipboardText.append(UploadList::instance()->at(row)->descr());
         clipboardText.append("\n");
     }
     if (!clipboardText.isEmpty()) {
         clipboard->setText(clipboardText);
     }
}

void TabWidget::updateApiKey() {
    Window::instance().settings.setValue("api_key", m_ui->lineEdit_api->text());
}

void TabWidget::loadOptions() {
    if( Window::instance().settings.value("system_tray_enabled").toBool()) {
        m_ui->checkBox_systemtray->setCheckState(Qt::Checked);
        toggleSystemTray(Qt::Checked);
    }
    m_ui->lineEdit_api->setText(Window::instance().settings.value("api_key").toString());
    m_ui->label_version->setText(VERSION);
}

void TabWidget::toggleSystemTray(int state) {
    bool tray_enabled = false;
    if (state == Qt::Checked)
        tray_enabled = true;
    QApplication::setQuitOnLastWindowClosed(!tray_enabled);
    Window::instance().trayIcon->setVisible(tray_enabled);
    Window::instance().settings.setValue("system_tray_enabled", tray_enabled);
}

void TabWidget::debugLog(QString message)  {
    m_ui->logsBrowser->appendPlainText(message);
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
