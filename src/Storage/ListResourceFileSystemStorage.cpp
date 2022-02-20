/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ListResourceFileSystemStorage.h"
#include <QDir>
#include <QJsonDocument>
#include <QDebug>
#include "FileSystemPaths.h"
ListResourceFileSystemStorage::ListResourceFileSystemStorage(QString qualifiedResourceName, QObject *parent) :
    IListResourceStorage(parent),
    _file(FileSystemPaths::instance()->getStoragePath()+qualifiedResourceName+".json"),
    _qualifiedResourceName(qualifiedResourceName)
{
    load();
}

bool ListResourceFileSystemStorage::appendItem(QVariant data)
{
    _listData.append(data);
    return save();
}

bool ListResourceFileSystemStorage::insertAt(QVariant data, IListResourceStorage::ItemUID item)
{
    _listData.insert(item.index, data);
    return save();
}

bool ListResourceFileSystemStorage::appendList(QVariantList data)
{
    _listData.append(data);
    return save();
}

bool ListResourceFileSystemStorage::removeItem(IListResourceStorage::ItemUID item)
{
    int idx = checkAndCorrectIndex(item);
    _listData.removeAt(idx);
    return save();
}

bool ListResourceFileSystemStorage::deleteList()
{
    _listData.clear();
    _metadata.clear();
    return _file.remove();
}

bool ListResourceFileSystemStorage::clearList()
{
    _listData.clear();
    return save();
}

bool ListResourceFileSystemStorage::set(QVariant data, IListResourceStorage::ItemUID item)
{
    int idx = checkAndCorrectIndex(item);
    _listData.replace(idx, data);
    return save();
}

bool ListResourceFileSystemStorage::setProperty(QString property, QVariant data, IListResourceStorage::ItemUID item)
{
    int idx = checkAndCorrectIndex(item);
    QVariantMap itemToModifiy = _listData.at(idx).toMap();
    itemToModifiy[property] = data;
    _listData.replace(idx, itemToModifiy);
    return save();
}

bool ListResourceFileSystemStorage::sync()
{
    return save();
}

bool ListResourceFileSystemStorage::setMetadata(QVariant metadata)
{
    _metadata = metadata.toMap();
    return save();
}

QVariantList ListResourceFileSystemStorage::getList() const
{
    return _listData;
}

QVariant ListResourceFileSystemStorage::getItem(ItemUID uid) const
{
    int index = checkAndCorrectIndex(uid);
    return _listData.at(index);
}

QVariant ListResourceFileSystemStorage::getMetadata() const
{
    return _metadata;
}

int ListResourceFileSystemStorage::getCount() const
{
    return _listData.count();
}

bool ListResourceFileSystemStorage::isReady() const
{
    return true;
}

int ListResourceFileSystemStorage::checkAndCorrectIndex(IListResourceStorage::ItemUID uid) const
{
    int index = uid.index;
    QString uuid = uid.uuid;

    if(_listData.isEmpty())
        return -1;

    if(index >= 0 && _listData.count() > index)
    {
        if(uuid.isEmpty())
            return index;

        QVariantMap item = _listData.at(index).toMap();
        if(item["uuid"].toString() == uuid)
        {
            return index;
        }
    }

    // need to search the appropriate index
    QListIterator<QVariant> it(_listData);
    int i = 0;
    while(it.hasNext())
    {
        if(it.next().toMap()["uuid"].toString() == uuid)
        {
            return i;
        }
        i++;
    }

    return -1;
}

bool ListResourceFileSystemStorage::save()
{
    QVariantMap data;
    data["listdata"] = _listData;
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

void ListResourceFileSystemStorage::load()
{
    if( _file.open(QFile::ReadOnly))
    {
        QVariantMap file =  QJsonDocument::fromJson(_file.readAll()).toVariant().toMap();
        _file.close();
        _listData = file["listdata"].toList();
        _metadata = file["metadata"].toMap();
    }
    else
    {
        qWarning()<<"Warning: Could not open File:  "<<_qualifiedResourceName<<" - "<<_file.errorString();
    }
}
