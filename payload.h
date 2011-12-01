#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <QIODevice>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QtScript>
#include <QScriptEngine>

class Payload : public QIODevice
{
public:
    Payload(QFile * file, QScriptValue & script_value);
    qint64 size() const;
    qint64 pos() const;
    qint64 position;
    QIODevice::OpenMode openMode() const;
    qint64 bytesAvailable() const;
    bool open(QIODevice::OpenMode mode);
    void close();
    bool atEnd() const;
    bool canReadLine() const;
    bool isSequential() const;
    static QString cont_disp_str, boundary, endline, start_delim, stop_delim;
    QByteArray payload_start;
    QFile * upload_file;
    QFileInfo upload_file_info;
    QSettings settings;
protected:
    qint64 readData( char * data, qint64 maxSize );
    qint64 writeData(const char *data, qint64 len);
    void addFormParam(const char * name, const char * value);
};

#endif // PAYLOAD_H
