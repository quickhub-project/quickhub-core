/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include <QDebug>
#include "ListResourceFactory.h"
#include "ListResource.h"
#include "Storage/ListResourceFileSystemStorage.h"

ListResourceFactory::ListResourceFactory(IListResourceStorageFactory* storageFactory, QObject *parent) : IResourceFactory(parent),
    _alternativeStorageFactory(storageFactory)
{
}

ListResourceFactory::ListResourceFactory(QObject *parent) : IResourceFactory(parent)
{
}

QString ListResourceFactory::getResourceType() const
{
    return "synclist";
}

resourcePtr ListResourceFactory::createResource(QString token, QString descriptor, QObject *parent)
{
    Q_UNUSED(parent)

    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
        return resourcePtr();

    QString resourceName = generateQualifiedResourceName(descriptor, token);

    IListResourceStorage* storage = nullptr;

    // default implementation
    if(nullptr == _alternativeStorageFactory)
    {
        qInfo()<< "Create ListResource with external storage plugin.";
        storage = _alternativeStorageFactory->createInstance(resourceName, nullptr);
    }
    else
    {
        storage = new ListResourceFileSystemStorage(resourceName, nullptr);
        qInfo()<<"Create ListResource with FS Resource Handler"<<resourceName;
    }

    return resourcePtr(new ListResource(storage));
}

void ListResourceFactory::setAlternativeStorageFactory(IListResourceStorageFactory *newAlternativeStorageFactory)
{
    _alternativeStorageFactory = newAlternativeStorageFactory;
}
