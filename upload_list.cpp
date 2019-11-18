#include "upload_list.h"

UploadList * UploadList::shared_instance = NULL;

UploadList * UploadList::instance() {
    if (shared_instance == NULL)
        shared_instance = new UploadList();
    return shared_instance;
}

UploadList::UploadList() : QObject(), mutex(), upload_list(){
    current_uploading = NULL;
}
UploadList::~UploadList(){}

void UploadList::add(Uploading * new_uploading)
{
    mutex.lock();
    for ( const auto& uploading : upload_list  )
        if (uploading->path() == new_uploading->path() && uploading->state() == "new") {
            delete new_uploading;
            mutex.unlock();
            return;
        }
    upload_list.push_front(new_uploading);
    mutex.unlock();
    emit changedList();
}

void UploadList::add(const QString & path) {
    add(new Uploading(path));
}

int UploadList::size() {
    return upload_list.size();
}

void UploadList::clear() {
    mutex.lock();
    for ( const auto& uploading : upload_list  )
        if (uploading != current_uploading)
            delete uploading;
    upload_list.clear();

    if (current_uploading != NULL)
        upload_list.append(current_uploading);
    mutex.unlock();
    emit changedList();
}


void UploadList::fail(QString message) {
    mutex.lock();
    current_uploading->descr(message);
    current_uploading->state(QObject::tr("error"));
    current_uploading = NULL;
    mutex.unlock();
    emit changedCurrent();
}

void UploadList::fail_all_pending(QString message) {
    mutex.lock();
    for ( const auto& uploading : upload_list  )
    if (uploading->state() == "new") {
        uploading->descr(message);
        uploading->state(QObject::tr("error"));
    }
    mutex.unlock();
    emit changedList();
}


void UploadList::stop_current_uploading() {
    if (current_uploading == NULL)
        return;
    mutex.lock();
    if (current_uploading->state() == "uploading")
        current_uploading->state(QObject::tr("stopped"));
    current_uploading = NULL;
    mutex.unlock();
    emit changedCurrent();
}

Uploading * UploadList::new_current_uploading(){
    mutex.lock();
    if (current_uploading == NULL)
      for ( const auto& uploading : upload_list  )
          if (uploading->state() == "new") {
              current_uploading = uploading;
              mutex.unlock();
              emit changedCurrent();
              return current_uploading;
          }
    mutex.unlock();
    return NULL;
}

void UploadList::change_current_state(QString state, QString description){
    mutex.lock();
    if (current_uploading != NULL) {
        current_uploading->state(state);
        current_uploading->descr(description);
    }
    mutex.unlock();
    emit changedCurrent();
}
Uploading * UploadList::at(int position){
    return upload_list.at(position);
}
