/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "FileSystemPaths.h"
#include <QObject>
#include <QDebug>

Q_GLOBAL_STATIC(FileSystemPaths, fsStorageManager);

FileSystemPaths *FileSystemPaths::instance()
{
    return fsStorageManager;
}

QString FileSystemPaths::getStoragePath()
{
    return _storagePath;
}

const QString &FileSystemPaths::getConfigPath() const
{
    return _configPath;
}

void FileSystemPaths::setConfigPath(const QString &newConfigPath)
{
    _configPath = newConfigPath;
}

void FileSystemPaths::setStoragePath(const QString &newStoragePath)
{
    _storagePath = newStoragePath;
}

FileSystemPaths::FileSystemPaths(QObject* parent) : QObject(parent)
{
}
