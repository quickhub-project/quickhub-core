/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "DeviceHandleHandlerFactory.h"
#include "DeviceHandleHandler.h"
#include "Server/Devices/DeviceManager.h"


DeviceHandleHandlerFactory::DeviceHandleHandlerFactory(QObject* parent) : IResourceHandlerFactory(parent)
{
}

IResourceHandler *DeviceHandleHandlerFactory::createInstance(QString descriptor, QString token, Err::CloudError *error)
{
    Q_UNUSED (token)

    deviceHandlePtr handle = DeviceManager::instance()->getHandleByMapping(descriptor);
    DeviceHandleHandler* handler = new DeviceHandleHandler(handle);

    if(error)
        *error = Err::NO_ERROR;

    return handler;
}

QString DeviceHandleHandlerFactory::resourceTypeIdentifier() const
{
    return "device";
}

QString DeviceHandleHandlerFactory::getResourceID(QString descriptor, QString token) const
{
    Q_UNUSED(token)
    return descriptor;
}
