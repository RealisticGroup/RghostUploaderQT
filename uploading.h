#ifndef UPLOADING_H
#define UPLOADING_H
#include <QStringList>

class Uploading : public QStringList
{
public:
    Uploading();
    Uploading(const QString & path);
    QString path();
    QString descr();
    QString state();
    void path(const QString & path);
    void descr(const QString & descr);
    void state(const QString & state);
};

#endif // UPLOADING_H
