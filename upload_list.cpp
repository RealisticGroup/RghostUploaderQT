#include "upload_list.h"
#include <QtGlobal>
Uploading * UploadList::current_uploading;
QList<Uploading *> UploadList::upload_list;
QMutex UploadList::mutex;

UploadList::UploadList(){
    current_uploading = NULL;
}
UploadList::~UploadList(){}

void UploadList::add(Uploading * uploading)
{
    mutex.lock();
    for (int j = 0; j < upload_list.size(); ++j) {
        if (upload_list.at(j)->path() == uploading->path() && upload_list.at(j)->state() == "new") {
            delete uploading;
            mutex.unlock();
            return;
        }
    }

    upload_list.push_front(uploading);
    mutex.unlock();
}

void UploadList::add(const QString & path) {
    add(new Uploading(path));
}

int UploadList::size() {
    return upload_list.size();
}

void UploadList::clear() {
    mutex.lock();
    for (int i = 0; i < size(); ++i) {
        if (upload_list.at(i) != current_uploading)
            delete upload_list.at(i);
    }
    upload_list.clear();

    if (current_uploading != NULL)
        upload_list.append(current_uploading);
    mutex.unlock();
}

Uploading * UploadList::new_current_uploading(){
    Uploading * new_uploading = NULL;
    mutex.lock();
    if (current_uploading == NULL)
      for (int j = 0; j < size(); ++j)
          if (upload_list.at(j)->state() == "new") {
              new_uploading = upload_list.at(j);
              current_uploading = new_uploading;
              break;
          }
    mutex.unlock();
    return new_uploading;
}

Uploading * UploadList::at(int position){
    return upload_list.at(position);
}
