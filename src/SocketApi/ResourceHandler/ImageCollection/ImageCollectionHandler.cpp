/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ImageCollectionHandler.h"

ImageCollectionHandler::ImageCollectionHandler(QSharedPointer<ImageResource> resource, QObject *parent) : IResourceHandler("imgcoll", parent),
    _resource( resource)
{
    connect(_resource.data(), &ImageResource::imageAdded, this, &ImageCollectionHandler::imageAddedSlot);
}

void ImageCollectionHandler::initHandle(ISocket *handle)
{
    QVariantMap msg;
    msg["command"] = "imgcoll:dump";
    QVariantMap parameters;
    parameters["data"] = _resource->getAllMetadata();
    msg["parameters"] = parameters;
    handle->sendVariant(msg);
}

void ImageCollectionHandler::handleMessage(QVariant message, ISocket *handle)
{
    Q_UNUSED(message);
    Q_UNUSED(handle);

}

void ImageCollectionHandler::imageAddedSlot(QString uuid)
{
    QVariantMap msg;
    msg["command"] = "imgcoll:new";
    QVariantMap item;
    item["metadata"] =  _resource->getMetaData(uuid);
    item["uid"] = uuid;
    QVariantMap parameters;
    parameters["data"] = item;
    msg["parameters"] = parameters;
    deployToAll(msg);
}
