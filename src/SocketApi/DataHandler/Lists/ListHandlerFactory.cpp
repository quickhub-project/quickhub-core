/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "ListHandlerFactory.h"
#include "ListHandler.h"
#include "ListWrapper/DeviceListWrapper.h"
#include "ListWrapper/UserListWrapper.h"
#include "ListWrapper/DeviceHandleListWrapper.h"

ListHandlerFactory::ListHandlerFactory(QObject *parent) : IResourceHandlerFactory(parent){}

IResourceHandler *ListHandlerFactory::createInstance(QString descriptor, QString token, Err::CloudError *error)
{

    Q_UNUSED (token)
    if(error)
        *error = Err::NO_ERROR;

    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
        return nullptr;

    if(descriptor == "users")
    {
        if(!(user->isAuthorizedTo(MONITOR_USERS)))
        {
            *error = Err::PERMISSION_DENIED;
            return nullptr;
        }

        IList* userList = new UserListWrapper();
        ListHandler* userListHandler = new ListHandler(userList);
        userList->setParent(userListHandler);
        return userListHandler;
    }


    if(descriptor == "devices")
    {
        if(!(user->isAuthorizedTo(MANAGE_DEVICES)))
        {
            *error = Err::PERMISSION_DENIED;
            return nullptr;
        }

        IList* deviceList = new DeviceListWrapper();
        ListHandler* deviceListHandler = new ListHandler(deviceList);
        deviceList->setParent(deviceListHandler);
        return deviceListHandler;
    }

    if(descriptor == "deviceHandles")
    {
        if(!(user->isAuthorizedTo(SEE_DEVICES)))
        {
            *error = Err::PERMISSION_DENIED;
            return nullptr;
        }
        IList* deviceList = new DeviceHandleListWrapper();
        ListHandler* deviceListHandler = new ListHandler(deviceList);
        deviceList->setParent(deviceListHandler);
        return deviceListHandler;
    }

    if(error)
        *error = Err::INVALID_DESCRIPTOR;

    return nullptr;
}

QString ListHandlerFactory::getResourceID(QString descriptor, QString token) const
{
    Q_UNUSED(token)
    return descriptor;
}

QString ListHandlerFactory::resourceTypeIdentifier() const
{
    return "list";
}
