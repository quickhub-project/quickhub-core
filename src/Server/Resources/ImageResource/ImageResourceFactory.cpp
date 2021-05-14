/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ImageResourceFactory.h"

#include "ImageResourceFactory.h"
#include "ImageResource.h"
#include "PluginManager.h"
#include "IImageResourceStorageFactory.h"
#include "../src/Storage/ImageResourceFilesystemStorage.h"


ImageResourceFactory::ImageResourceFactory(QObject *parent) : IResourceFactory(parent)
{
    
}

QString ImageResourceFactory::getResourceType() const
{
    return "imgcoll";
}

resourcePtr ImageResourceFactory::createResource(QString token, QString descriptor, QObject *parent)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
        return resourcePtr();

    QString resourceName = generateQualifiedResourceName(descriptor, token);
    QList<IImageResourceStorageFactory*> storagePlugins = PluginManager::getInstance()->getObjects<IImageResourceStorageFactory>();
    IImageResourceStorage* storage = nullptr;
    if(storagePlugins.count() > 0)
    {
        qInfo()<< "Create ImageResource with external storage plugin.";
        storage = storagePlugins[0]->createInstance(resourceName, nullptr);
    }
    else //fallback implementation
        storage = new ImageResourceFilesystemStorage(resourceName, nullptr);

    return resourcePtr(new ImageResource(storage, parent));
}
