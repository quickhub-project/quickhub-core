/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "ListResource.h"
#include <QJsonDocument>
#include <QDateTime>
#include <QDir>
#include <QUuid>
#include <QDebug>
#include <QFileInfo>

#include "IListResourceStorage.h"
#include "../../Authentication/AuthentificationService.h"

ListResource::ListResource(IListResourceStorage *storage, QObject *parent) : IResource("", parent),
    _lastAccess(QDateTime::currentMSecsSinceEpoch()),
    _listStorage(storage)
{
    if(storage)
        storage->setParent(this);
}

ListResource::~ListResource()
{
    if(!_listStorage)
        return;
    _mutex.lockForWrite();
    _listStorage->sync();
    _mutex.unlock();
}

qint64 ListResource::lastAccess() const
{
    QReadLocker locker(&_mutex);
    return _lastAccess;
}

QVariantList ListResource::getListData() const
{
    if(!_listStorage)
        return QVariantList();

    QReadLocker locker(&_mutex);
    return _listStorage->getList();
}

QVariant ListResource::getItem(int idx, QString uuid) const
{
    if(!_listStorage)
        return QVariant();
    IListResourceStorage::ItemUID uid;
    uid.index = idx;
    uid.uuid = uuid;
    QReadLocker locker(&_mutex);
    return _listStorage->getItem(uid);
}

QVariantMap ListResource::getMetadata() const
{

    if(!_listStorage)
        return QVariantMap();

    QReadLocker locker(&_mutex);
    return _listStorage->getMetadata().toMap();
}

int ListResource::getCount() const
{
    if(!_listStorage)
        return -1;

    return _listStorage->getCount();
}

ListResource::ModificationResult ListResource::appendItem(QVariant data, QString token)
{
    qDebug().noquote()<<QJsonDocument::fromVariant(data).toJson();
    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);

    if(identity.isNull() || !_allowUserAccess)
    {
        ModificationResult result;
        result.error = PERMISSION_DENIED;
        return result;
    }

    return appendItem(data, identity);
}

IResource::ModificationResult ListResource::appendItem(QVariant data, iIdentityPtr user)
{
    ModificationResult result;
    QVariantMap item = prepareTemplate(user);
    item["data"] = data;
    result.data = item;
    _mutex.lockForWrite();
    if(_listStorage && _listStorage->appendItem(item))
    {
        _lastAccess = QDateTime::currentMSecsSinceEpoch();
        Q_EMIT itemAppended(item, user);
    }
    else
    {
        result.error = STORAGE_ERROR;
    }
    _mutex.unlock();
    return result;

}

ListResource::ModificationResult ListResource::insertAt(QVariant data, int index, QString token)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull() || !_allowUserAccess)
    {
        ModificationResult result;
        result.error =  PERMISSION_DENIED;
        return result;
    }

    return insertAt(data, index, user);
}

IResource::ModificationResult ListResource::insertAt(QVariant data, int index, iIdentityPtr user)
{
    ModificationResult result;
    if(index < 0)
    {
        result.error =  INVALID_PARAMETERS;
        return result;
    }

    QVariantMap item = prepareTemplate(user);
    item["data"] = data;
    result.data = item;
    _mutex.lockForWrite();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    IListResourceStorage::ItemUID uid;
    uid.index = index;
    if(_listStorage->insertAt(item, uid))
        Q_EMIT itemInserted(item, index, user);
    else
        result.error = STORAGE_ERROR;

    _mutex.unlock();
    return result;
}

ListResource::ModificationResult ListResource::appendList(QVariantList data, QString token)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);

    if(user.isNull() || !_allowUserAccess)
    {
        ModificationResult result;
        result.error =  PERMISSION_DENIED;
        return result;
    }

    return appendList(data, user);
}

ListResource::ModificationResult ListResource::appendList(QVariantList data, iIdentityPtr user)
{
    ModificationResult result;
    QVariantList itemsToAppend;
    QListIterator<QVariant> it(data);

    while(it.hasNext())
    {
        QVariantMap item = prepareTemplate(user);
        item["data"] = it.next();
        itemsToAppend.append(item);
    }

    result.data = itemsToAppend;
    _mutex.lockForWrite();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    if(_listStorage->appendList(itemsToAppend))
        Q_EMIT listAppended(itemsToAppend, user);
    else
        result.error = STORAGE_ERROR;
    _mutex.unlock();
    return result;
}

void ListResource::resetData(QVariantList data, iIdentityPtr user)
{
    QVariantList itemsToAppend;
    QListIterator<QVariant> it(data);

    while(it.hasNext())
    {
        QVariantMap item = prepareTemplate(user);
        item["data"] = it.next();
        itemsToAppend.append(item);
    }

    _mutex.lockForWrite();
    _listStorage->clearList();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    if(_listStorage->appendList(itemsToAppend))
        Q_EMIT reset();
    _mutex.unlock();
}


ListResource::ModificationResult ListResource::removeItem(QString uuid, QString token, int index)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);

    if(user.isNull() || !_allowUserAccess)
    {
        ModificationResult result;
        result.error =  PERMISSION_DENIED;
        return result;
    }

    return removeItem(index, user, uuid);
}

IResource::ModificationResult ListResource::removeItem(int index, iIdentityPtr user, QString uuid)
{
    ModificationResult result;
    IListResourceStorage::ItemUID uid;
    uid.index = index;
    uid.uuid = uuid;

    _mutex.lockForWrite();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    if(_listStorage && _listStorage->removeItem(uid))
        Q_EMIT itemRemoved(index, uuid, user);
    else
        result.error = STORAGE_ERROR;
    _mutex.unlock();
    return result;
}


ListResource::ModificationResult ListResource::deleteList(QString token)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull() || !_allowUserAccess)
    {
        ModificationResult result;
        result.error =  PERMISSION_DENIED;
        return result;
    }

    return deleteList(user);
}

IResource::ModificationResult ListResource::deleteList(iIdentityPtr user)
{
    ModificationResult result;
    _mutex.lockForWrite();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    if(_listStorage && _listStorage->deleteList())
    {
        Q_EMIT listDeleted(user);
    }
    else
    {
        result.error = STORAGE_ERROR;
    }
    _mutex.unlock();
    return result;
}

ListResource::ModificationResult ListResource::clearList(QString token)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull() || !_allowUserAccess)
    {
        ModificationResult result;
        result.error =  PERMISSION_DENIED;
        return result;
    }

    return clearList(user);

}

IResource::ModificationResult ListResource::clearList(iIdentityPtr user)
{
    ModificationResult result;
    _mutex.lockForWrite();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    if(_listStorage && _listStorage->clearList())
    {
        Q_EMIT listCleared(user);
    }
    else
    {
        result.error = STORAGE_ERROR;
    }

    _mutex.unlock();
    return result;
}


ListResource::ModificationResult ListResource::set(QVariant data, int index, QString uuid, QString token)
{
    ModificationResult result;
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);

    if(user.isNull() | !_allowUserAccess)
    {
        result.error =  PERMISSION_DENIED;
        return result;
    }   

    return set(data, index, user, uuid);
}

IResource::ModificationResult ListResource::set(QVariant data, int index, iIdentityPtr user, QString uuid)
{
    IListResourceStorage::ItemUID uid;
    uid.index = index;
    uid.uuid = uuid;
    ModificationResult result;
    if(!_listStorage)
    {
        result.error =  STORAGE_ERROR;
        return result;
    }

    QVariantMap item =  _listStorage->getItem(uid).toMap();
    item["lastupdate"] = QDateTime::currentMSecsSinceEpoch();
    item["data"] = data;

    if(!user.isNull())
    {
        item["userid"] = user->identityID();
       // item["username"] = user->userName();
    }

    _mutex.lockForWrite();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    bool success = _listStorage->set(item, uid);
    _mutex.unlock();

    if(!success)
    {
        result.error = INVALID_PARAMETERS;
        return result;
    }

    result.data = item;
    Q_EMIT itemSet(item, index, uuid, user);
    _listStorage->sync();
    return result;

}


ListResource::ModificationResult ListResource::setProperty(QString property, QVariant data, int index, QString uuid, QString token)
{

    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);

    if(user.isNull() || !_allowUserAccess)
    {
        ModificationResult result;
        result.error =  PERMISSION_DENIED;
        return result;
    }

    return setProperty(property, data, index, user, uuid);
}


IResource::ModificationResult ListResource::setProperty(QString property, QVariant data, int index, iIdentityPtr user, QString uuid)
{
    ModificationResult result;
    if(!_listStorage)
    {
        result.error =  STORAGE_ERROR;
        return result;
    }

    if(index < 0)
    {
        result.error = INVALID_PARAMETERS;
        return result;
    }

    IListResourceStorage::ItemUID uid;
    uid.index = index;
    uid.uuid = uuid;

    // get item to modify
    QVariantMap item = _listStorage->getItem(uid).toMap();
    if(!user.isNull())
    {
        item["userid"] = user->identityID();
       // item["username"] = user->userName();
    }

    //modifiy item -> set new timestamp
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    item["lastupdate"] = timestamp;
    // get payload of item
    QVariantMap tmp = item["data"].toMap();
    //set new property value
    tmp[property] = data;
    item["data"] = tmp;
    //add modified value to return data
    result.data = item;
    _mutex.lockForWrite();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    if(_listStorage->set(item, uid))
        Q_EMIT propertySet(property, item, index, uuid, user, timestamp);
    else
        result.error = STORAGE_ERROR;
    _mutex.unlock();
    return result;
}

void ListResource::setStorage(IListResourceStorage *storage)
{
    storage->setParent(this);
    _listStorage = storage;
   // Q_EMIT reset();  not sure if this is a good idea
}

void ListResource::setAllowUserAccess(bool enabled)
{
    _allowUserAccess = enabled;
}


ListResource::ModificationResult ListResource::setMetadata(QVariant metadata)
{
    ListResource::ModificationResult result;
    _mutex.lockForWrite();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    if(_listStorage &&  _listStorage->setMetadata(metadata))
        Q_EMIT metadataChanged();
    else
        result.error = STORAGE_ERROR;

    _mutex.unlock();
    return result;
}

bool ListResource::setFilter(QVariantMap query  )
{
    Q_UNUSED(query)
    return false;
}

bool ListResource::isPermittedToRead(QString token) const
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    return !user.isNull();
}

bool ListResource::isPermittedToWrite(QString token) const
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    return !user.isNull();
}

const QString ListResource::getResourceType() const
{
    return "synclist";
}

QVariantMap ListResource::prepareTemplate(iIdentityPtr user) const
{
    QVariantMap item;
    if(!user.isNull())
    {
        item["userid"] = user->identityID();
    }
    item["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    item["uuid"] = createUUID();
    return item;
}

QString ListResource::createUUID() const
{
    return QUuid::createUuid().toString();
}


