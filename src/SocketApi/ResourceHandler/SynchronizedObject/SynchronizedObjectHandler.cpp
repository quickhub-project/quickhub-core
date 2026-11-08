/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "SynchronizedObjectHandler.h"
#include "Server/Authentication/User.h"
#include "Server/Authentication/AuthentificationService.h"
#include <QMap>
SynchronizedObjectHandler::SynchronizedObjectHandler(QSharedPointer<ObjectResource> resource) : IResourceHandler(resource->getResourceType(), resource.data()),
    _resource(resource),
    _proxy(new ObjectAccessProxy(resource, this))
{
    connect(_resource.data(), &ObjectResource::propertyChanged, this, &SynchronizedObjectHandler::propertyChanged);
    connect(_resource.data(), &ObjectResource::sendEvent, this, &SynchronizedObjectHandler::sendEvent);
}

SynchronizedObjectHandler::~SynchronizedObjectHandler()
{
}

void SynchronizedObjectHandler::initHandle(ISocket *handle)
{
    QVariantMap msg;
    msg["command"] = "object:dump";
    QVariantMap parameters;
    QString token = _tokenToHandleMap.key(handle);
    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    if(identity.isNull())
    {
        qWarning() << "SynchronizedObjectHandler::initHandle - invalid identity for handle";
        return;
    }
    parameters["data"] = _proxy->getObjectData(identity);
    parameters["metadata"] = _proxy->getMetaData();
    msg["parameters"] = parameters;
    handle->sendVariant(msg);
}

bool SynchronizedObjectHandler::dynamicContent() const
{
    return _resource->dynamicContent();
}

bool SynchronizedObjectHandler::isPermitted(QString token) const
{
    return _proxy->isPermittedToRead(AuthenticationService::instance()->validateToken(token));
}

void SynchronizedObjectHandler::propertyChanged(QString property, QVariant data, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "object:property:set";
    QVariantMap parameters;
    parameters["property"] = property;
    parameters["data"] =  data;
    msg["parameters"] = parameters;

    if (!_resource->hasPropertyFilter())
    {
        deployToAll(msg);
        return;
    }

    for (ISocket* handle : _handles)
    {
        QString handleToken = _tokenToHandleMap.key(handle);
        iIdentityPtr identity = AuthenticationService::instance()->validateToken(handleToken);
        if (!identity.isNull() && _proxy->canReadProperty(property, identity))
        {
            handle->sendVariant(msg);
        }
    }
}

void SynchronizedObjectHandler::handleMessage(QVariant message, ISocket *handle)
{
    QVariantMap msg     = message.toMap();

    QString     command     = msg["command"].toString();
    QString     token       = msg["token"].toString();
    QVariantMap parameters  = msg["parameters"].toMap();
    QVariant    data        = parameters["data"];
    msg.remove("token");

    if(command == "object:property:set")
    {
        QString property = parameters["property"].toString();
        QMap<QString,PropertyChangeEvent> events;
        QObject tmp;
        disconnect(_resource.data(), &ObjectResource::propertyChanged, this, &SynchronizedObjectHandler::propertyChanged);
        connect(_resource.data(), &ObjectResource::propertyChanged, &tmp, [&events](QString property, QVariant data, iIdentityPtr user){events.insert(property, PropertyChangeEvent{property, data, user});});
        ObjectResource::ModificationResult result = _proxy->setProperty(property, data, token);
        connect(_resource.data(), &ObjectResource::propertyChanged, this, &SynchronizedObjectHandler::propertyChanged);
        events.remove(property);

        parameters["data"] = result.data;
        msg["parameters"] = parameters;

        handleError(command, result.error, handle, parameters);
        if(result.error == ObjectResource::NO_ERROR)
        {
            deployToAll(msg, handle);
        }

        for (auto [key, value] : events.asKeyValueRange()) {
            qDebug()<<"Other Prop Changes:"<<value.property;
            propertyChanged(value.property, value.data, value.user);
        }
    }

    if(command == "object:filter")
    {
        if(_resource->dynamicContent())
        {
            _resource->setFilter(data.toMap());
        }
    }
}

void SynchronizedObjectHandler::sendEvent(QVariantMap data)
{
    QVariantMap msg;
    msg["command"] = "object:event";
    QVariantMap parameters;
    parameters["data"] =  data;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void SynchronizedObjectHandler::deployToAllFiltered(QVariantMap msg, std::function<QVariantMap(QVariantMap, iIdentityPtr)> filterFn)
{
    for (ISocket* handle : _handles)
    {
        QString handleToken = _tokenToHandleMap.key(handle);
        iIdentityPtr identity = AuthenticationService::instance()->validateToken(handleToken);
        if(identity.isNull())
            continue;
        QVariantMap filtered = filterFn(msg, identity);
        handle->sendVariant(filtered);
    }
}
