/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "FileSystemLoader.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>

// ----------------------------------------------------------------------------
// Logging
// ----------------------------------------------------------------------------

Q_LOGGING_CATEGORY(lcFileSystemLoader, "quickhub.filesystemloader")

// ----------------------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------------------

FileSystemLoader::FileSystemLoader(QString path, QObject *parent) : QObject(parent),
    _resourcePath(path),
    _file(path)
{
}

QVariantMap FileSystemLoader::load()
{
    if(! _file.exists()) {
        qCInfo(lcFileSystemLoader) << "Could not open file for loading:" << _resourcePath << "- File does not exist yet";
        return QVariantMap();
    }
    if( _file.open(QFile::ReadOnly))
    {
        QVariantMap file =  QJsonDocument::fromJson(_file.readAll()).toVariant().toMap();
        _file.close();
        return file;
    }
    else
    {
        qCWarning(lcFileSystemLoader) << "Could not open file for loading:" << _resourcePath << "-" << _file.errorString();
        return QVariantMap();
    }
}

bool FileSystemLoader::deleteFile()
{
    return _file.remove();
}

bool FileSystemLoader::save(QVariantMap data)
{
    QFileInfo info(_file);
    QDir dir(info.absolutePath());
    if(!dir.exists())
    {
        dir.mkpath(info.absolutePath());
    }

    if(_file.open(QFile::WriteOnly))
    {
        _file.write(QJsonDocument::fromVariant(data).toJson());
        _file.close();
    }
    else
    {
        qCWarning(lcFileSystemLoader) << "Could not open file for saving:" << _file.fileName() << "-" << _file.errorString();
        return false;
    }
    return true;
}
