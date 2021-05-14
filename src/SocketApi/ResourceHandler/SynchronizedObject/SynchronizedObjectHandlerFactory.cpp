/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "SynchronizedObjectHandlerFactory.h"
#include "SynchronizedObjectHandler.h"
#include "Server/Resources/ObjectResource/ObjectResource.h"
#include "Server/Resources/ResourceManager/ResourceManager.h"

SynchronizedObjectHandlerFactory::SynchronizedObjectHandlerFactory(QObject *parent) : IResourceHandlerFactory(parent){}

IResourceHandler *SynchronizedObjectHandlerFactory::createInstance(QString path, QString token, Err::CloudError *error)
{
    resourcePtr resource = ResourceManager::instance()->getOrCreateResource("object", path, token, error);
    QSharedPointer<ObjectResource> objectResource = resource.objectCast<ObjectResource>();
    if(objectResource.isNull())
        return nullptr;

    return new SynchronizedObjectHandler(objectResource);
}

QString SynchronizedObjectHandlerFactory::resourceTypeIdentifier() const
{
    return "object";
}

QString SynchronizedObjectHandlerFactory::getResourceID(QString descriptor, QString token) const
{
    return ResourceManager::instance()->getResourceID("object", descriptor, token);
}
