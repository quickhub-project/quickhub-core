/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "FileSystemStorageManager.h"
#include <QObject>
#include <QDebug>

Q_GLOBAL_STATIC(FileSystemStorageManager, fsStorageManager);

FileSystemStorageManager *FileSystemStorageManager::instance()
{
    return fsStorageManager;
}

void FileSystemStorageManager::init(QString path)
{
    _storagePath = path;
}

QString FileSystemStorageManager::getStoragePath()
{
    return _storagePath;
}

FileSystemStorageManager::FileSystemStorageManager(QObject* parent) : QObject(parent)
{
}
