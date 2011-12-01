#ifndef UPLOAD_LIST_H
#define UPLOAD_LIST_H
#include <QList>
#include <QString>
#include <QMutex>
#include "uploading.h"

class UploadList
{
public:
    static void add(Uploading * uploading);
    static void add(const QString & path);
    static void clear();
    static Uploading * new_current_uploading();
    static int size();
    static Uploading * current_uploading;
    static Uploading * at(int);
    static QList<Uploading *> upload_list;
    static QMutex mutex;
private:
    UploadList();
    ~UploadList();
};

#endif // UPLOAD_LIST_H
