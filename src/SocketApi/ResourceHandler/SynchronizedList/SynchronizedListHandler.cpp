/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "SynchronizedListHandler.h"
#include <QDateTime>
#include <QDir>
#include <QUuid>
#include <QJsonDocument>
#include "Server/Resources/ResourceManager/ResourceManager.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/User.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Resources/ListResource/ListAccessProxy.h"

SynchronizedListHandler::SynchronizedListHandler(QSharedPointer<ListResource> resource) : IResourceHandler(resource->getResourceType(), resource.data()),
    _resource(resource),
    _proxy(new ListAccessProxy(resource, this))
{
    connect(_resource.data(), &ListResource::itemAppended, this, &SynchronizedListHandler::itemAppended);
    connect(_resource.data(), &ListResource::itemInserted, this, &SynchronizedListHandler::itemInserted);
    connect(_resource.data(), &ListResource::listAppended, this, &SynchronizedListHandler::listAppended);
    connect(_resource.data(), &ListResource::listCleared,  this, &SynchronizedListHandler::listCleared);
    connect(_resource.data(), &ListResource::listDeleted,  this, &SynchronizedListHandler::listDeleted);
    connect(_resource.data(), &ListResource::metadataChanged,  this, &SynchronizedListHandler::metadataChanged);
    connect(_resource.data(), &ListResource::itemSet,      this, &SynchronizedListHandler::itemSet);
    connect(_resource.data(), &ListResource::itemRemoved,  this, &SynchronizedListHandler::itemRemoved);
    connect(_resource.data(), &ListResource::propertySet,  this, &SynchronizedListHandler::propertySet);
     connect(_resource.data(), &ListResource::reset,  this, &SynchronizedListHandler::listResetted);
}

SynchronizedListHandler::~SynchronizedListHandler()
{
}

void SynchronizedListHandler::initHandle(ISocket* handle)
{
    QVariantMap msg;
    QVariantMap parameters;

    if(_resource->getCount() > 0)
    {
        msg["command"] = "synclist:init";
        parameters["count"] = _resource->getCount();
    }
    else
    {
        msg["command"] = "synclist:dump";
        QString token = _tokenToHandleMap.key(handle);
        iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
        parameters["data"] = _proxy->getListData(identity);
    }

    parameters["metadata"] = _proxy->getMetadata();
    msg["parameters"] = parameters;
    handle->sendVariant(msg);
}

bool SynchronizedListHandler::dynamicContent() const
{
    return _resource->dynamicContent();
}

void SynchronizedListHandler::handleMessage(QVariant message, ISocket *handle)
{
    QVariantMap msg     = message.toMap();

    QString     command     = msg["command"].toString();
    QString     token       = msg["token"].toString();
    QVariantMap parameters  = msg["parameters"].toMap();
    QVariant    data        = parameters["data"];
    msg.remove("token");

    if(command == "synclist:dump")
    {
        QString dumpToken = _tokenToHandleMap.key(handle);
        iIdentityPtr dumpIdentity = AuthenticationService::instance()->validateToken(dumpToken);
        parameters["data"] = _proxy->getListData(dumpIdentity);
        parameters["metadata"] = _proxy->getMetadata();
        msg["parameters"] = parameters;
        handle->sendVariant(msg);
        return;
    }

    if(command == "synclist:get")
    {
        int from = parameters["from"].toInt();
        int count = parameters["count"].toInt();
        if(from < 0 || count <= 0 || from+count-1  >= _proxy->getCount())
        {
            return;
        }

        QString getToken = _tokenToHandleMap.key(handle);
        iIdentityPtr getIdentity = AuthenticationService::instance()->validateToken(getToken);
        QVariantList data;

        for(int i = from; i < from+count; i++)
        {
            data << _proxy->getItem(i, getIdentity);
        }

        parameters["data"] = data;

        msg["parameters"] = parameters;
        handle->sendVariant(msg);
        return;
    }

    if(command == "synclist:filter")
    {
        if(_proxy->dynamicContent())
            _proxy->setFilter(data.toMap());
    }

    // ############### MODIFIER

    if(command == "synclist:append")
    {
        disconnect(_resource.data(), &ListResource::itemAppended, this, &SynchronizedListHandler::itemAppended);
        ListResource::ModificationResult result = _proxy->appendItem(data, token);
        connect(_resource.data(), &ListResource::itemAppended, this, &SynchronizedListHandler::itemAppended);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            parameters["data"] = result.data;
            msg["parameters"] = parameters;
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "synclist:insertat")
    {
        bool ok;
        int index = parameters["index"].toInt(&ok);
        if(!ok)
        {
            qWarning()<<"SocketListHandler: Invalid data (index not an integer?)";
            return;
        }

        disconnect(_resource.data(), &ListResource::itemInserted, this, &SynchronizedListHandler::itemInserted);
        ListResource::ModificationResult result = _proxy->insertAt(data, index, token);
        connect(_resource.data(), &ListResource::itemInserted, this, &SynchronizedListHandler::itemInserted);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            parameters["data"] = result.data;
            msg["parameters"] = parameters;
            deployToAll(msg, handle);
            return;
        }
    }


    if(command == "synclist:appendlist")
    {
        QVariantList dataList = data.toList();

        disconnect(_resource.data(), &ListResource::listAppended, this, &SynchronizedListHandler::listAppended);
        ListResource::ModificationResult result = _proxy->appendList(dataList, token);
        connect(_resource.data(), &ListResource::listAppended, this, &SynchronizedListHandler::listAppended);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            parameters["data"] = result.data;
            msg["parameters"] = parameters;
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "synclist:clear")
    {
        disconnect(_resource.data(), &ListResource::listCleared, this, &SynchronizedListHandler::listCleared);
        ListResource::ModificationResult result = _proxy->clearList(token);
        connect(_resource.data(), &ListResource::listCleared, this, &SynchronizedListHandler::listCleared);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }


    if(command == "synclist:delete")
    {
        disconnect(_resource.data(), &ListResource::listDeleted, this, &SynchronizedListHandler::listDeleted);
        ListResource::ModificationResult result = _proxy->deleteList(token);
        connect(_resource.data(), &ListResource::listDeleted, this, &SynchronizedListHandler::listDeleted);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "synclist:property:set")
    {
        int index = parameters["index"].toInt();
        QString uuid = parameters["uuid"].toString();
        QString property = parameters["property"].toString();

        disconnect(_resource.data(), &ListResource::propertySet, this, &SynchronizedListHandler::propertySet);
        ListResource::ModificationResult result = _proxy->setProperty(property, data, index, uuid, token);
        connect(_resource.data(), &ListResource::propertySet, this, &SynchronizedListHandler::propertySet);

        parameters["lastupdate"] = result.data.toMap()["lastupdate"];
        parameters["userid"] = result.data.toMap()["userid"];
        parameters["username"] = result.data.toMap()["username"];

        msg["parameters"] = parameters;
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "synclist:set")
    {
        int index = parameters["index"].toInt();
        QString uuid = parameters["uuid"].toString();

        disconnect(_resource.data(), &ListResource::itemSet, this, &SynchronizedListHandler::itemSet);
        ListResource::ModificationResult result = _proxy->set(data, index, uuid, token);
        connect(_resource.data(), &ListResource::itemSet, this, &SynchronizedListHandler::itemSet);

        parameters["data"] = result.data;
        msg["parameters"] = parameters;
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }

    if(command == "synclist:metadata:set")
    {
         disconnect(_resource.data(), &ListResource::metadataChanged,  this, &SynchronizedListHandler::metadataChanged);
        _proxy->setMetadata(parameters["metadata"]);
        connect(_resource.data(), &ListResource::metadataChanged,  this, &SynchronizedListHandler::metadataChanged);
        deployToAll(msg, handle);
    }

    if(command == "synclist:remove")
    {
        int index = parameters["index"].toInt();
        QString uuid = parameters["uuid"].toString();

        disconnect(_resource.data(), &ListResource::itemRemoved, this, &SynchronizedListHandler::itemRemoved);
        ListResource::ModificationResult result = _proxy->removeItem(uuid, token, index);
        connect(_resource.data(), &ListResource::itemRemoved, this, &SynchronizedListHandler::itemRemoved);
        handleError(command, result.error, handle);
        if(result.error == ListResource::NO_ERROR)
        {
            deployToAll(msg, handle);
            return;
        }
    }
}



void SynchronizedListHandler::metadataChanged()
{
    QVariantMap msg;
    msg["command"] = "synclist:metadata:set";
    QVariantMap parameters;
    parameters["metadata"] =  _resource.data()->getMetadata();
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void SynchronizedListHandler::itemAppended(QVariant data, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:append";
    QVariantMap parameters;
    parameters["data"] =  data;
    msg["parameters"] = parameters;

    if (!_resource->hasPropertyFilter())
    {
        deployToAll(msg);
        return;
    }

    deployToAllFiltered(msg, [this](QVariantMap m, iIdentityPtr identity) {
        QVariantMap params = m["parameters"].toMap();
        params["data"] = _proxy->filterItemForRead(params["data"].toMap(), identity);
        m["parameters"] = params;
        return m;
    });
}

void SynchronizedListHandler::itemInserted(QVariant data, int index, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:insertat";
    QVariantMap parameters;
    parameters["data"] =  data;
    parameters["index"] =  index;
    msg["parameters"] = parameters;

    if (!_resource->hasPropertyFilter())
    {
        deployToAll(msg);
        return;
    }

    deployToAllFiltered(msg, [this](QVariantMap m, iIdentityPtr identity) {
        QVariantMap params = m["parameters"].toMap();
        params["data"] = _proxy->filterItemForRead(params["data"].toMap(), identity);
        m["parameters"] = params;
        return m;
    });
}

void SynchronizedListHandler::listAppended(QVariantList data, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:appendlist";
    QVariantMap parameters;
    parameters["data"] =  data;
    msg["parameters"] = parameters;

    if (!_resource->hasPropertyFilter())
    {
        deployToAll(msg);
        return;
    }

    deployToAllFiltered(msg, [this](QVariantMap m, iIdentityPtr identity) {
        QVariantMap params = m["parameters"].toMap();
        QVariantList items = params["data"].toList();
        QVariantList filtered;
        filtered.reserve(items.size());
        for (const QVariant& item : items)
            filtered.append(_proxy->filterItemForRead(item.toMap(), identity));
        params["data"] = filtered;
        m["parameters"] = params;
        return m;
    });
}

void SynchronizedListHandler::itemRemoved(int index, QString uuid, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:remove";
    QVariantMap parameters;
    parameters["uuid"] =  uuid;
    parameters["index"] =  index;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void SynchronizedListHandler::listDeleted(iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:delete";
    deployToAll(msg);
}

void SynchronizedListHandler::listCleared(iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:clear";
    deployToAll(msg);
}

void SynchronizedListHandler::itemSet(QVariant data, int index, QString uuid, iIdentityPtr user)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:set";
    QVariantMap parameters;
    parameters["uuid"] =  uuid;
    parameters["index"] =  index;
    parameters["data"] =  data;
    msg["parameters"] = parameters;

    if (!_resource->hasPropertyFilter())
    {
        deployToAll(msg);
        return;
    }

    deployToAllFiltered(msg, [this](QVariantMap m, iIdentityPtr identity) {
        QVariantMap params = m["parameters"].toMap();
        params["data"] = _proxy->filterItemForRead(params["data"].toMap(), identity);
        m["parameters"] = params;
        return m;
    });
}

void SynchronizedListHandler::propertySet(QString property, QVariant data, int index, QString uuid, iIdentityPtr user, qint64 timestamp)
{
    Q_UNUSED(user)
    QVariantMap msg;
    msg["command"] = "synclist:property:set";
    QVariantMap parameters;
    parameters["uuid"] =  uuid;
    parameters["index"] =  index;
    parameters["property"] =  property;
    if(data.typeId() == QMetaType::QVariantMap)
        parameters["data"] =  data.toMap()["data"].toMap()[property];
    else
        parameters["data"] = data;

    parameters["lastupdate"] = timestamp;
    if(!user.isNull())
    {
        parameters["userid"] = user->identityID();
        //parameters["username"] = user->userName();
    }
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
        if (_proxy->canReadProperty(property, identity))
        {
            handle->sendVariant(msg);
        }
    }
}

void SynchronizedListHandler::listResetted()
{
    QVariantMap msg;
    QVariantMap parameters;
    msg["command"] = "synclist:init";
    parameters["count"] =  _resource->getCount();
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void SynchronizedListHandler::deployToAllFiltered(QVariantMap msg, std::function<QVariantMap(QVariantMap, iIdentityPtr)> filterFn)
{
    for (ISocket* handle : _handles)
    {
        QString handleToken = _tokenToHandleMap.key(handle);
        iIdentityPtr identity = AuthenticationService::instance()->validateToken(handleToken);
        QVariantMap filtered = filterFn(msg, identity);
        handle->sendVariant(filtered);
    }
}

bool SynchronizedListHandler::isPermitted(QString token) const
{
    return  _resource->isPermittedToRead(AuthenticationService::instance()->validateToken(token));
}
