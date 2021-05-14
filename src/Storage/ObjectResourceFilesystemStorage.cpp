/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ObjectResourceFilesystemStorage.h"
#include <QJsonDocument>
#include <QDebug>
#include <QDir>
#include "FileSystemStorageManager.h"

ObjectResourceFilesystemStorage::ObjectResourceFilesystemStorage(QString qualifiedResourceName, QObject *parent):
    IObjectResourceStorage(parent),
    _file(FileSystemStorageManager::instance()->getStoragePath()+qualifiedResourceName+".json"),
    _qualifiedResourceName(qualifiedResourceName)
{
    load();
}

bool ObjectResourceFilesystemStorage::insertProperty(QString name, QVariant value)
{
    _propertyData.insert(name, value);
    return save();
}

bool ObjectResourceFilesystemStorage::sync()
{
    return save();
}

bool ObjectResourceFilesystemStorage::setMetadata(QVariant metadata)
{
    _metadata = metadata;
    return save();
}

QVariant ObjectResourceFilesystemStorage::getProperty(QString name) const
{
    return _propertyData[name];
}

QVariantMap ObjectResourceFilesystemStorage::getAllProperties() const
{
    return _propertyData;
}

QVariant ObjectResourceFilesystemStorage::getMetadata() const
{
    return _metadata;
}

bool ObjectResourceFilesystemStorage::save()
{
    QVariantMap data;
    data["properties"] = _propertyData;
    data["metadata"] = _metadata;

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
        return true;
    }
    else
    {
        qWarning()<<"Warning: Could not open file -"<<_file.errorString();
        return false;
    }
}

void ObjectResourceFilesystemStorage::load()
{
    if( _file.open(QFile::ReadOnly))
    {
        QVariantMap file =  QJsonDocument::fromJson(_file.readAll()).toVariant().toMap();
        _file.close();
        _propertyData = file["properties"].toMap();
        _metadata = file["metadata"].toMap();
    }
    else
    {
        qWarning()<<"Warning: Could not open File:  "<<_qualifiedResourceName<<" - "<<_file.errorString();
    }
}

