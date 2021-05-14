/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ImageResource.h"
#include "IImageResourceStorage.h"
#include <QDateTime>
#include <QUuid>

ImageResource::ImageResource(IImageResourceStorage* storage, QObject* parent): IResource("", parent),
_listStorage(storage)
{
}

qint64 ImageResource::lastAccess() const
{
    return _lastAccess;
}

const QString ImageResource::getResourceType() const
{
    return "imgcoll";
}

IResource::ModificationResult ImageResource::insert(QImage image, QVariant data, QString id, QString token)
{
    ModificationResult result;
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);

    if(user.isNull())
    {
        result.error = PERMISSION_DENIED;
        return result;
    }

    QVariantMap item = prepareTemplate(user);
    item["data"] = data;
    result.data = item;

    _lock.lockForWrite();
    bool success = _listStorage->insertImage(image, item, id);
    _lock.unlock();

    if(!success)
        result.error = STORAGE_ERROR;
    else
        Q_EMIT imageAdded(id);

    return result;
}

IResource::ModificationResult ImageResource::deleteImage(QString uid, QString token)
{
    ModificationResult result;

    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);

    if(user.isNull())
    {
        result.error = PERMISSION_DENIED;
        return result;
    }

    _lock.lockForWrite();
    bool success =_listStorage->deleteImage(uid);
    _lock.unlock();

    if(!success)
        result.error = STORAGE_ERROR;

    return result;
}

QStringList ImageResource::getAllImageIds(QString token)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);

    if(user.isNull())
    {
        //todo error handling
        return QStringList();
    }
    QReadLocker locker(&_lock);
    return _listStorage->getAllImageIds();
}

QImage ImageResource::getImage(QString id, QString token)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);

    if(user.isNull())
    {
        //todo error handling
        return QImage();
    }

    QReadLocker locker(&_lock);
    return _listStorage->getImage(id);
}

QVariant ImageResource::getMetaData(QString id)
{
    QReadLocker locker(&_lock);
    return _listStorage->getMetadata(id);
}

QVariantMap ImageResource::getAllMetadata()
{
    QReadLocker locker(&_lock);
    return _listStorage->getAllMetadata();
}

QVariantMap ImageResource::prepareTemplate(iIdentityPtr user) const
{
    QVariantMap item;
    item["userid"] = user->identityID();
    item["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    item["uuid"] = QUuid::createUuid().toString();
    return item;
}
