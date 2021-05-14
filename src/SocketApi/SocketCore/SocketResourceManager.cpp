/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include <QJsonDocument>

#include "SocketResourceManager.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/User.h"
#include "Server/Resources/ResourceManager/ResourceManager.h"

SocketResourceManager::SocketResourceManager(QObject *parent) : IRequestHandler(parent)
{
}

SocketResourceManager::~SocketResourceManager()
{
    QMapIterator<QString, QMap<QString, IResourceHandler*>*> it(_resourceHandler);
    while(it.hasNext())
        delete it.next().value();
}

bool SocketResourceManager::handleRequest(QVariantMap message, ISocket *handle)
{
    QString command             = message["command"].toString();

    if(!getSupportedCommands().contains(command))
        return false;

    QStringList commandTokens   = command.split(":");
    QString     resourceType    = commandTokens[0];
    QString     token           = message["token"].toString();

    if(!_resourceHandler.contains(resourceType) | !_handlerFactorys.contains(resourceType))
        return false;

    QVariantMap payload       =  message["payload"].toMap();
    QString descriptor        =  payload["descriptor"].toString();
    QString UUID              = _handlerFactorys[resourceType]->getResourceID(descriptor, token);

    IResourceHandler* handler = nullptr;
    Err::CloudError error = Err::NO_ERROR;


    if(!_resourceHandler[resourceType]->contains(UUID))
    {
        handler = _handlerFactorys[resourceType]->createInstance(descriptor, token, &error);
        if(handler && error == Err::NO_ERROR)
        {
            handler->setParent(this);
            handler->setProperty("resourceType", resourceType);
            connect(handler, &IResourceHandler::destroyed, this, &SocketResourceManager::handlerDeleted);
            if(!handler->dynamicContent())
                 _resourceHandler[resourceType]->insert(UUID, handler);
        }
        if(!handler)
        {
            QVariantMap answer;
            answer["command"] = command+":failed";
            answer["errorstring"] = "No resource available.";
            handle->sendVariant(answer);
            return false;
        }
    }
    else
    {
        handler = _resourceHandler[resourceType]->value(UUID);
    }

    if(handler)
    {
        //backend will reparent the socket!
        if(handler->attachHandle(token, handle))
            return true;

        QVariantMap answer;
        answer["command"] = command+":failed";
        answer["errorstring"] = "Token invalid. Please log in and try again.";
        handle->sendVariant(answer);
        return false;
    }

    QVariantMap answer;
    answer["command"] = command+":failed";
    answer["errorstring"] = "Unknown error.";
    handle->sendVariant(answer);
    return false;
 }

QStringList SocketResourceManager::getSupportedCommands()
{
    return _supportedCommands;
}

void SocketResourceManager::init(QString storageDirectory)
{
    _dataStoragePath = storageDirectory;
}

void SocketResourceManager::handlerDeleted(QObject* obj)
{
    IResourceHandler* handler = static_cast<IResourceHandler*>(obj);
    QString type = handler->property("resourceType").toString();
    if(!type.isEmpty())
    {
        _resourceHandler[type]->remove(_resourceHandler[type]->key(handler));
        // TODO: Delete Resource?
    }
}

void SocketResourceManager::registerFactory(IResourceHandlerFactory *factory)
{
    QString identifier = factory->resourceTypeIdentifier();
    QMap<QString, IResourceHandler*>*  map = new QMap<QString, IResourceHandler*>();
    _resourceHandler.insert(identifier, map);
    _handlerFactorys.insert(identifier, factory);
    _supportedCommands << identifier+":attach";
}
