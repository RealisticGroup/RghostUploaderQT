#include "uploading.h"

Uploading::Uploading(){
    this->append("");
    this->append("");
    this->append("new");
}

Uploading::Uploading(const QString & path){
    this->append(path);
    this->append("");
    this->append("new");
}

QString Uploading::path(){
    return this->at(0);
}
QString Uploading::descr(){
    return this->at(1);
}
QString Uploading::state(){
    return this->at(2);
}
void Uploading::path(const QString & path){
    this->replace(0, path);
}
void Uploading::descr(const QString & descr){
    this->replace(1, descr);
}
void Uploading::state(const QString & state){
    this->replace(2, state);
}
