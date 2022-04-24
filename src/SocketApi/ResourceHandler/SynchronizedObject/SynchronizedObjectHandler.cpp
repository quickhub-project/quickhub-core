/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "SynchronizedObjectHandler.h"
#include "Server/Authentication/User.h"

SynchronizedObjectHandler::SynchronizedObjectHandler(QSharedPointer<ObjectResource> resource) : IResourceHandler(resource->getResourceType(), resource.data()),
    _resource(resource)
{
    connect(_resource.data(), &ObjectResource::propertyChanged, this, &SynchronizedObjectHandler::propertyChanged);
}

SynchronizedObjectHandler::~SynchronizedObjectHandler()
{
}

void SynchronizedObjectHandler::initHandle(ISocket *handle)
{
    QVariantMap msg;
    msg["command"] = "object:dump";
    QVariantMap parameters;
    parameters["data"] = _resource->getObjectData();
    parameters["metadata"] = _resource->getMetaData();
    msg["parameters"] = parameters;
    handle->sendVariant(msg);
}

bool SynchronizedObjectHandler::dynamicContent() const
{
    return _resource->dynamicContent();
}

bool SynchronizedObjectHandler::isPermitted(QString token) const
{
    return _resource->isPermittedToRead(token);
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
    deployToAll(msg);
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
        disconnect(_resource.data(), &ObjectResource::propertyChanged, this, &SynchronizedObjectHandler::propertyChanged);
        ObjectResource::ModificationResult result = _resource->setProperty(property, data, token);
        connect(_resource.data(), &ObjectResource::propertyChanged, this, &SynchronizedObjectHandler::propertyChanged);

        parameters["data"] = result.data;
        msg["parameters"] = parameters;

        handleError(command, result.error, handle, parameters);
        if(result.error == ObjectResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "object:filter")
    {
        if(_resource->dynamicContent())
            _resource->setFilter(data.toMap());
    }
}
