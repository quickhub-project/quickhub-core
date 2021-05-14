/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ObjectResourceFactory.h"
#include "ObjectResource.h"
#include "IObjectResourceStorageFactory.h"
#include "PluginManager.h"
#include "Storage/ObjectResourceFilesystemStorage.h"


ObjectResourceFactory::ObjectResourceFactory(QObject* parent) : IResourceFactory(parent)
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
    QList<IObjectResourceStorageFactory*> storagePlugins = PluginManager::getInstance()->getObjects<IObjectResourceStorageFactory>();
    IObjectResourceStorage* storage = nullptr;
    if(storagePlugins.count() > 0)
    {
        qInfo()<< "Create ObjectResource with external storage plugin.";
        storage = storagePlugins[0]->createInstance(resourceName, nullptr);
    }
    else //fallback implementation
    {
        storage = new ObjectResourceFilesystemStorage(resourceName, nullptr);
        qInfo()<<"Create ObjectResource with FS Resource Handler  "<<resourceName;
    }

    return resourcePtr(new ObjectResource(storage, parent));
}
