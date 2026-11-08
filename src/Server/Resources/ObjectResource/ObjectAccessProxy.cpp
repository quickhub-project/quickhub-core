/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ObjectAccessProxy.h"
#include "../../Authentication/AuthentificationService.h"

ObjectAccessProxy::ObjectAccessProxy(QSharedPointer<ObjectResource> resource, QObject* parent)
    : QObject(parent),
      _resource(resource)
{
}

QVariantMap ObjectAccessProxy::getObjectData(iIdentityPtr identity) const
{
    QVariantMap data = _resource->getObjectData();

    if (!_resource->hasPropertyFilter())
        return data;

    return filterObjectForRead(data, identity);
}

QVariantMap ObjectAccessProxy::getMetaData() const
{
    return _resource->getMetaData();
}

IResource::ModificationResult ObjectAccessProxy::setProperty(QString name, const QVariant& value, QString token)
{
    if (!_resource->hasPropertyFilter())
        return _resource->setProperty(name, value, token);

    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    if (!canWriteProperty(name, identity))
    {
        IResource::ModificationResult result;
        result.error = IResource::PERMISSION_DENIED;
        return result;
    }

    return _resource->setProperty(name, value, token);
}

QVariantMap ObjectAccessProxy::filterObjectForRead(const QVariantMap& objectData, iIdentityPtr identity) const
{
    if (!_resource->hasPropertyFilter())
        return objectData;

    QVariantMap filtered;
    QMapIterator<QString, QVariant> it(objectData);
    while (it.hasNext())
    {
        it.next();
        if (canReadProperty(it.key(), identity))
            filtered.insert(it.key(), it.value());
    }
    return filtered;
}

bool ObjectAccessProxy::canReadProperty(const QString& property, iIdentityPtr identity) const
{
    if (!_resource->hasPropertyFilter())
        return true;

    return _resource->propertyFilter()(identity, property).canRead;
}

bool ObjectAccessProxy::canWriteProperty(const QString& property, iIdentityPtr identity) const
{
    if (!_resource->hasPropertyFilter())
        return true;

    return _resource->propertyFilter()(identity, property).canWrite;
}

ObjectResource* ObjectAccessProxy::resource() const
{
    return _resource.data();
}

QSharedPointer<ObjectResource> ObjectAccessProxy::resourcePtr() const
{
    return _resource;
}

bool ObjectAccessProxy::isPermittedToRead(iIdentityPtr identity) const
{
    return _resource->isPermittedToRead(identity);
}

bool ObjectAccessProxy::isPermittedToWrite(iIdentityPtr identity) const
{
    return _resource->isPermittedToWrite(identity);
}

bool ObjectAccessProxy::dynamicContent() const
{
    return _resource->dynamicContent();
}

bool ObjectAccessProxy::setFilter(QVariantMap query)
{
    return _resource->setFilter(query);
}
