/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "SynchronizedListHandlerFactory.h"
#include "SynchronizedListHandler.h"
#include "Server/Resources/ResourceManager/ResourceManager.h"

SynchronizedListHandlerFactory::SynchronizedListHandlerFactory(QObject* parent)  : IResourceHandlerFactory(parent){}

IResourceHandler *SynchronizedListHandlerFactory::createInstance(QString path, QString token, Err::CloudError* error)
{
    resourcePtr resource = ResourceManager::instance()->getOrCreateResource("synclist", path, token, error);
    QSharedPointer<ListResource> listResource = resource.objectCast<ListResource>();

    if(listResource.isNull())
    {
        return nullptr;
    }

    return new SynchronizedListHandler(listResource);
}

QString SynchronizedListHandlerFactory::resourceTypeIdentifier() const
{
    return "synclist";
}

QString SynchronizedListHandlerFactory::getResourceID(QString descriptor, QString token) const
{
    return ResourceManager::instance()->getResourceID("synclist", descriptor, token);
}
