/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ListAccessProxy.h"
#include "../../Authentication/AuthentificationService.h"

ListAccessProxy::ListAccessProxy(QSharedPointer<ListResource> resource, QObject* parent)
    : QObject(parent),
      _resource(resource)
{
}

QVariantList ListAccessProxy::getListData(iIdentityPtr identity) const
{
    QVariantList list = _resource->getListData();

    if (!_resource->hasPropertyFilter())
        return list;

    QVariantList filtered;
    filtered.reserve(list.size());
    for (const QVariant& item : list)
    {
        filtered.append(filterItemForRead(item.toMap(), identity));
    }
    return filtered;
}

QVariant ListAccessProxy::getItem(int idx, iIdentityPtr identity, QString uuid) const
{
    QVariant item = _resource->getItem(idx, uuid);

    if (!_resource->hasPropertyFilter())
        return item;

    return filterItemForRead(item.toMap(), identity);
}

QVariantMap ListAccessProxy::getMetadata() const
{
    return _resource->getMetadata();
}

int ListAccessProxy::getCount() const
{
    return _resource->getCount();
}

IResource::ModificationResult ListAccessProxy::appendItem(QVariant data, QString token)
{
    if (!_resource->hasPropertyFilter())
        return _resource->appendItem(data, token);

    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    QVariantMap dataMap = data.toMap();
    QMapIterator<QString, QVariant> it(dataMap);
    while (it.hasNext())
    {
        it.next();
        if (!canWriteProperty(it.key(), identity))
        {
            IResource::ModificationResult result;
            result.error = IResource::PERMISSION_DENIED;
            return result;
        }
    }

    return _resource->appendItem(data, token);
}

IResource::ModificationResult ListAccessProxy::insertAt(QVariant data, int index, QString token)
{
    if (!_resource->hasPropertyFilter())
        return _resource->insertAt(data, index, token);

    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    QVariantMap dataMap = data.toMap();
    QMapIterator<QString, QVariant> it(dataMap);
    while (it.hasNext())
    {
        it.next();
        if (!canWriteProperty(it.key(), identity))
        {
            IResource::ModificationResult result;
            result.error = IResource::PERMISSION_DENIED;
            return result;
        }
    }

    return _resource->insertAt(data, index, token);
}

IResource::ModificationResult ListAccessProxy::appendList(QVariantList data, QString token)
{
    if (!_resource->hasPropertyFilter())
        return _resource->appendList(data, token);

    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    for (const QVariant& item : data)
    {
        QVariantMap dataMap = item.toMap();
        QMapIterator<QString, QVariant> it(dataMap);
        while (it.hasNext())
        {
            it.next();
            if (!canWriteProperty(it.key(), identity))
            {
                IResource::ModificationResult result;
                result.error = IResource::PERMISSION_DENIED;
                return result;
            }
        }
    }

    return _resource->appendList(data, token);
}

IResource::ModificationResult ListAccessProxy::setProperty(QString property, QVariant data, int index, QString uuid, QString token)
{
    if (!_resource->hasPropertyFilter())
        return _resource->setProperty(property, data, index, uuid, token);

    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    if (!canWriteProperty(property, identity))
    {
        IResource::ModificationResult result;
        result.error = IResource::PERMISSION_DENIED;
        return result;
    }

    return _resource->setProperty(property, data, index, uuid, token);
}

IResource::ModificationResult ListAccessProxy::set(QVariant data, int index, QString uuid, QString token)
{
    if (!_resource->hasPropertyFilter())
        return _resource->set(data, index, uuid, token);

    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    QVariantMap dataMap = data.toMap();
    QMapIterator<QString, QVariant> it(dataMap);
    while (it.hasNext())
    {
        it.next();
        if (!canWriteProperty(it.key(), identity))
        {
            IResource::ModificationResult result;
            result.error = IResource::PERMISSION_DENIED;
            return result;
        }
    }

    return _resource->set(data, index, uuid, token);
}

IResource::ModificationResult ListAccessProxy::removeItem(QString uuid, QString token, int index)
{
    return _resource->removeItem(uuid, token, index);
}

IResource::ModificationResult ListAccessProxy::deleteList(QString token)
{
    return _resource->deleteList(token);
}

IResource::ModificationResult ListAccessProxy::clearList(QString token)
{
    return _resource->clearList(token);
}

IResource::ModificationResult ListAccessProxy::setMetadata(QVariant metadata)
{
    return _resource->setMetadata(metadata);
}

bool ListAccessProxy::setFilter(QVariantMap query)
{
    return _resource->setFilter(query);
}

QVariantMap ListAccessProxy::filterItemForRead(const QVariantMap& item, iIdentityPtr identity) const
{
    if (!_resource->hasPropertyFilter())
        return item;

    QVariantMap filtered = item;
    QVariantMap data = item["data"].toMap();
    QVariantMap filteredData;
    QMapIterator<QString, QVariant> it(data);
    while (it.hasNext())
    {
        it.next();
        if (canReadProperty(it.key(), identity))
            filteredData.insert(it.key(), it.value());
    }
    filtered["data"] = filteredData;
    return filtered;
}

bool ListAccessProxy::canReadProperty(const QString& property, iIdentityPtr identity) const
{
    if (!_resource->hasPropertyFilter())
        return true;

    return _resource->propertyFilter()(identity, property).canRead;
}

bool ListAccessProxy::canWriteProperty(const QString& property, iIdentityPtr identity) const
{
    if (!_resource->hasPropertyFilter())
        return true;

    return _resource->propertyFilter()(identity, property).canWrite;
}

ListResource* ListAccessProxy::resource() const
{
    return _resource.data();
}

QSharedPointer<ListResource> ListAccessProxy::resourcePtr() const
{
    return _resource;
}

bool ListAccessProxy::isPermittedToRead(iIdentityPtr identity) const
{
    return _resource->isPermittedToRead(identity);
}

bool ListAccessProxy::isPermittedToWrite(iIdentityPtr identity) const
{
    return _resource->isPermittedToWrite(identity);
}

bool ListAccessProxy::dynamicContent() const
{
    return _resource->dynamicContent();
}
