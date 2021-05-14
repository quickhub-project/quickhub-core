/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef FILESYSTEMSTORAGEMANAGER_H
#define FILESYSTEMSTORAGEMANAGER_H

#include <QObject>

class FileSystemStorageManager : public QObject
{
    Q_OBJECT

public:
    FileSystemStorageManager(QObject* parent = nullptr);
    static FileSystemStorageManager* instance();
    void init(QString path);
    QString getStoragePath();

private:
    QString                             _storagePath;

};

#endif // FILESYSTEMSTORAGEMANAGER_H
