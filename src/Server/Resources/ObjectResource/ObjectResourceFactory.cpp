/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "IObjectResourceStorageFactory.h"
#include "ObjectResource.h"
#include "Storage/ObjectResourceFilesystemStorage.h"
#include "../ResourceManager/IResourceFactory.h"

ObjectResourceFactory::ObjectResourceFactory(QObject* parent) : IResourceFactory(parent)
{
}

ObjectResourceFactory::ObjectResourceFactory(IObjectResourceStorageFactory *storageFactory, QObject *parent) : IResourceFactory(parent),
    _alternativeStorageFactory(storageFactory)
{

}

QString ObjectResourceFactory::getResourceType() const
{
   return "object";
}


resourcePtr ObjectResourceFactory::createResource(QString token, QString descriptor, QObject *parent)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
        return resourcePtr();

    QString resourceName = generateQualifiedResourceName(descriptor, token);

    IObjectResourceStorage* storage = nullptr;

    // default implementation
    if(nullptr == _alternativeStorageFactory)
    {
        qInfo()<< "Create ObjectResource with external storage plugin.";
        storage = _alternativeStorageFactory->createInstance(resourceName, nullptr);
    }
    else
    {
        storage = new ObjectResourceFilesystemStorage(resourceName, nullptr);
        qInfo()<<"Create ObjectResource with FS Resource Handler  "<<resourceName;
    }

    return resourcePtr(new ObjectResource(storage, parent));
}

void ObjectResourceFactory::setAlternativeStorageFactory(IObjectResourceStorageFactory *newAlternativeStorageFactory)
{
    _alternativeStorageFactory = newAlternativeStorageFactory;
}
