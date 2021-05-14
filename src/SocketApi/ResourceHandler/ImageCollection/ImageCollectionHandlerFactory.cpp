/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ImageCollectionHandlerFactory.h"
#include "Server/Resources/ResourceManager/ResourceManager.h"
#include "Server/Resources/ImageResource/ImageResource.h"
#include "ImageCollectionHandler.h"

ImageCollectionHandlerFactory::ImageCollectionHandlerFactory(QObject *parent) : IResourceHandlerFactory(parent)
{

}

QString ImageCollectionHandlerFactory::getResourceID(QString descriptor, QString token) const
{
    return ResourceManager::instance()->getResourceID("synclist", descriptor, token);
}

QString ImageCollectionHandlerFactory::resourceTypeIdentifier() const
{
    return "imgcoll";
}

IResourceHandler *ImageCollectionHandlerFactory::createInstance(QString descriptor, QString token, Err::CloudError *error)
{
    Q_UNUSED(error);
    resourcePtr resource = ResourceManager::instance()->getOrCreateResource("imgcoll", descriptor, token);
    QSharedPointer<ImageResource> imgResource = resource.objectCast<ImageResource>();
    if(!resource)
        return nullptr;

    return new ImageCollectionHandler(imgResource);
}
