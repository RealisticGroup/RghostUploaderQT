#ifndef UPLOAD_LIST_H
#define UPLOAD_LIST_H
#include <QObject>
#include <QList>
#include <QString>
#include <QMutex>
#include "uploading.h"

class UploadList : public QObject
{
Q_OBJECT

public:
    UploadList();
    static UploadList * instance();
    static UploadList * shared_instance;

    QMutex mutex;
    QList<Uploading *> upload_list; //  = QList<Uploading*>();

    Uploading * current_uploading;

    void add(Uploading * uploading);
    void add(const QString & path);
    void clear();
    Uploading * new_current_uploading();
    Uploading * at(int);
    int size();
    void fail(QString message);
    void fail_all_pending(QString message);
    void stop_current_uploading();
    void change_current_state(QString state, QString description);

signals:
    void changedCurrent();
    void changedList();

private:
    ~UploadList();
};

#endif // UPLOAD_LIST_H
