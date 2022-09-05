/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ImageResourceFactory.h"
#include "ImageResource.h"
#include "IImageResourceStorageFactory.h"
#include "Storage/ImageResourceFilesystemStorage.h"

ImageResourceFactory::ImageResourceFactory(IImageResourceStorageFactory*  storageFactory, QObject *parent) : IResourceFactory(parent),
    _alternativeStorageFactory(storageFactory)
{
    
}

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

    IImageResourceStorage* storage = nullptr;

    // default implementation
    if(nullptr == _alternativeStorageFactory)
    {
        qInfo()<< "Create ListResource with external storage plugin.";
        storage = _alternativeStorageFactory->createInstance(resourceName, nullptr);
    }
    else
    {
        storage = new ImageResourceFilesystemStorage(resourceName, nullptr);
        qInfo()<<"Create ListResource with FS Resource Handler"<<resourceName;
    }

    return resourcePtr(new ImageResource(storage, parent));
}

void ImageResourceFactory::setAlternativeStorageFactory(IImageResourceStorageFactory *newAlternativeStorageFactory)
{
    _alternativeStorageFactory = newAlternativeStorageFactory;
}
