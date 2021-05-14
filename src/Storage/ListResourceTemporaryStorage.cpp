/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ListResourceTemporaryStorage.h"
#include <QDir>
#include <QJsonDocument>
#include <QDebug>


ListResourceTemporaryStorage::ListResourceTemporaryStorage(QObject *parent) :
    IListResourceStorage(parent)
{
}

bool ListResourceTemporaryStorage::appendItem(QVariant data)
{
    _listData.append(data);
    return true;
}

bool ListResourceTemporaryStorage::insertAt(QVariant data, IListResourceStorage::ItemUID item)
{
    _listData.insert(item.index, data);
    return true;
}

bool ListResourceTemporaryStorage::appendList(QVariantList data)
{
    _listData.append(data);
    return true;
}

bool ListResourceTemporaryStorage::removeItem(IListResourceStorage::ItemUID item)
{
    int idx = checkAndCorrectIndex(item);
    _listData.removeAt(idx);
    return true;
}

bool ListResourceTemporaryStorage::deleteList()
{
    _listData.clear();
    _metadata.clear();
    return true;
}

bool ListResourceTemporaryStorage::clearList()
{
    _listData.clear();
    return true;
}

bool ListResourceTemporaryStorage::set(QVariant data, IListResourceStorage::ItemUID item)
{
    int idx = checkAndCorrectIndex(item);
    _listData.replace(idx, data);
    return true;
}

bool ListResourceTemporaryStorage::setProperty(QString property, QVariant data, IListResourceStorage::ItemUID item)
{
    int idx = checkAndCorrectIndex(item);
    QVariantMap itemToModifiy = _listData.at(idx).toMap();
    itemToModifiy[property] = data;
    _listData.replace(idx, itemToModifiy);
    return true;
}

bool ListResourceTemporaryStorage::sync()
{
    return true;
}

bool ListResourceTemporaryStorage::setMetadata(QVariant metadata)
{
    _metadata = metadata.toMap();
    return true;
}

QVariantList ListResourceTemporaryStorage::getList() const
{
    return _listData;
}

QVariant ListResourceTemporaryStorage::getItem(ItemUID uid) const
{
    int index = checkAndCorrectIndex(uid);
    return _listData.at(index);
}

QVariant ListResourceTemporaryStorage::getMetadata() const
{
    return _metadata;
}

int ListResourceTemporaryStorage::getCount() const
{
    return _listData.count();
}

bool ListResourceTemporaryStorage::isReady() const
{
    return true;
}

int ListResourceTemporaryStorage::checkAndCorrectIndex(IListResourceStorage::ItemUID uid) const
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
