/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "ObjectResource.h"
#include <QJsonDocument>
#include <QDateTime>
#include <QDir>
#include <QUuid>
#include <QDebug>
#include <QFileInfo>

#include "../../Authentication/AuthentificationService.h"
#include "IObjectResourceStorage.h"

ObjectResource::ObjectResource(IObjectResourceStorage* storage, QObject *parent) : IResource("", parent),
    _lastAccess(QDateTime::currentMSecsSinceEpoch()),
    _storage(storage)
{
    if(storage != nullptr)
        storage->setParent(this);
}

ObjectResource::~ObjectResource()
{
    if(_storage == nullptr)
        return;

    _mutex.lockForWrite();
    _storage->sync();
    _mutex.unlock();
}

qint64 ObjectResource::lastAccess() const
{
    return _lastAccess;
}

const QString ObjectResource::getResourceType() const
{
    return "object";
}

QVariantMap ObjectResource::getObjectData() const
{
    if(_storage == nullptr)
        return QVariantMap();

    QReadLocker locker(&_mutex);
    return _storage->getAllProperties();
}

QVariantMap ObjectResource::getMetaData() const
{
    if(_storage == nullptr)
        return QVariantMap();

    QReadLocker locker(&_mutex);
    return _storage->getMetadata().toMap();
}

ObjectResource::ModificationResult ObjectResource::setProperty(QString name, const QVariant &value, QString token)
{
    ModificationResult result;
    iIdentityPtr  user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
    {
        result.error = PERMISSION_DENIED;
        return result;
    }

    QVariantMap data;
    data["data"] = value;
    data["userid"] = user->identityID();
    data["lastupdate"] = QDateTime::currentMSecsSinceEpoch();
    _mutex.lockForWrite();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    _storage->insertProperty(name, data);
    Q_EMIT propertyChanged(name, value, user);
    _mutex.unlock();
    result.data = value;
    return result;
}


bool ObjectResource::setFilter(QVariantMap query)
{
    Q_UNUSED(query)
    return false;
}

