#include "payload.h"
QString Payload::cont_disp_str = "Content-Disposition: form-data; ";
QString Payload::boundary = "RghostUploadBoundaryabcdef0123456789";
QString Payload::endline = "\r\n";
QString Payload::start_delim = "--" + Payload::boundary + Payload::endline;
QString Payload::stop_delim = Payload::endline + "--" + Payload::boundary + "--" + Payload::endline;

Payload::Payload(QFile * file, QScriptValue & script_value) : upload_file(file), upload_file_info(QFileInfo(*file))
{
    position = 0;
    const QString file_str = start_delim + cont_disp_str + "name=" + "\"file\"" + "; filename=" + "\"" +
                             upload_file_info.fileName() + "\""
                             + endline + "Content-Transfer-Encoding: binary" + endline + endline;

    addFormParam("authenticity_token", script_value.property("authenticity_token").toString().toLatin1());
    payload_start.append(file_str.toUtf8());
    open(QIODevice::ReadOnly);
}

qint64 Payload::size() const{
    return payload_start.size() + upload_file->size() + stop_delim.size();
}

qint64 Payload::bytesAvailable() const{
    return size() - position;
}

bool Payload::open(QIODevice::OpenMode mode){
    setOpenMode(mode);
    return true;
}
void Payload::close(){
    return;
}

bool Payload::atEnd () const {
    return bytesAvailable() == 0;
}

bool Payload::isSequential() const {
    return false;
}

bool Payload::canReadLine() const {
    return false;
}

qint64 Payload::pos() const {
    return position;
}

qint64 Payload::readData ( char * data, qint64 maxSize ) {
    QByteArray chunk;
    if (position < payload_start.size())
        chunk.append(payload_start.mid(position, maxSize));
    while(chunk.size() < maxSize && !upload_file->atEnd())
        chunk.append(upload_file->read(maxSize - chunk.size()));
    if (chunk.size() < maxSize)
        chunk.append(stop_delim.mid(stop_delim.size() - (size() - position - chunk.size()), maxSize - chunk.size()));
    position += chunk.size();
    memcpy(data, chunk.data(), chunk.size());
    return chunk.size();
}
qint64 Payload::writeData ( const char * data, qint64 len ) {
    return 0;
}

void Payload::addFormParam(const char * name, const char * value) {
    QString param = start_delim + cont_disp_str + "name=" + "\"" + name + "\"" + endline + endline + value + endline;
    payload_start.append(param);
}
