/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ImageResourceFilesystemStorage.h"
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include "FileSystemPaths.h"

ImageResourceFilesystemStorage::ImageResourceFilesystemStorage(QString qualifiedResourceName, QObject *parent) : IImageResourceStorage(parent),
    _file(FileSystemPaths::instance()->getStoragePath()+qualifiedResourceName+"/pictureCollection.json"),
    _resourceName(qualifiedResourceName)
{   

}

bool ImageResourceFilesystemStorage::insertImage(QImage image, QVariantMap metadata, QString uid)
{
    load();
    // _metadata is the persisted index - _images is only an in-memory cache and is
    // empty after this storage has been recreated for a new request.
    if(!_metadata.contains(uid))
    {
        // also creates the storage directory for the image file written below
        save();
        QString filename = FileSystemPaths::instance()->getStoragePath()+ _resourceName+"/"+uid;
        if(image.save(filename))
            qInfo()<<"File saved!" << filename;
        else
        {
            qCritical()<<"Could not save file: "<<filename;
        }

        _images.insert(uid, image);
        _metadata.insert(uid, metadata);
        return save();
    }
    else
        return false;
}

bool ImageResourceFilesystemStorage::deleteImage(QString uid)
{
    load();
    if(!_metadata.contains(uid))
        return false;

    _images.remove(uid);
    _metadata.remove(uid);

    // Without this the image file would survive and getImage() would keep serving it
    QString filename = FileSystemPaths::instance()->getStoragePath()+_resourceName+"/"+uid;
    if(QFile::exists(filename) && !QFile::remove(filename))
        qCritical()<<"Could not remove file: "<<filename;

    return save();
}

QStringList ImageResourceFilesystemStorage::getAllImageIds()
{
    load();
    return _metadata.keys();
}

QVariant ImageResourceFilesystemStorage::getMetadata(QString uid)
{
    load();
    return _metadata.value(uid).toMap();
}

QVariantMap ImageResourceFilesystemStorage::getAllMetadata()
{
    load();
    return _metadata;
}

QImage ImageResourceFilesystemStorage::getImage(QString uid)
{
    QString path = FileSystemPaths::instance()->getStoragePath()+_resourceName+"/"+uid;
    if(QFile::exists(path))
        return QImage(path);

    return QImage();
}

bool ImageResourceFilesystemStorage::load()
{
    if(_contentLoaded)
        return true;

    _contentLoaded = true;
    if( _file.open(QFile::ReadOnly))
    {
        QVariantList file =  QJsonDocument::fromJson(_file.readAll()).toVariant().toList();
        _file.close();
        for(int i = 0; i < file.size(); i++)
        {
            QVariantMap item = file[i].toMap();
            _metadata.insert(item["id"].toString(), item["metadata"]);
        }

        return true;
    }
    else
    {
        qWarning()<<"Warning: Could not open File:  "<<_resourceName<<" - "<<_file.errorString();
        return false;
    }
}

bool ImageResourceFilesystemStorage::save()
{
    QVariantList list;
    QMapIterator<QString, QVariant> it(_metadata);
    while(it.hasNext())
    {
        it.next();
        QVariantMap item;
        item["id"] = it.key();
        item["metadata"] = it.value();
        list <<item;
    }

    QFileInfo info(_file);
    QDir dir(info.absolutePath());
    if(!dir.exists())
    {
        dir.mkpath(info.absolutePath());
    }

    if(_file.open(QFile::WriteOnly))
    {
        _file.write(QJsonDocument::fromVariant(list).toJson());
        _file.close();
        return true;
    }
    else
    {
        qWarning()<<"Warning: Could not open file -"<<_file.errorString();
        return false;
    }
}
