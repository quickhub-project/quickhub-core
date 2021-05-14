/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "DevicePermissionManager.h"

Q_GLOBAL_STATIC(DevicePermissionManager, devicePermissionManager);

DevicePermissionManager::DevicePermissionManager(QObject *parent) : QObject(parent)
{

}

DevicePermissionManager *DevicePermissionManager::instance()
{
    return devicePermissionManager;
}

bool DevicePermissionManager::registerPermissionChecker(devicePermissionCheckerPtr checker)
{
    QStringList controlledTypes = checker->getControlledDeviceTypes();
    QListIterator<QString> typeIt(controlledTypes);

    while(typeIt.hasNext())
    {
        QString type = typeIt.next();
        if(_devicePermissionControllerMap.contains(type))
            return false;
    }

    while(typeIt.hasPrevious())
    {
        QString type = typeIt.previous();
        _devicePermissionControllerMap.insert(type, checker);
    }

    return true;
}

devicePermissionCheckerPtr DevicePermissionManager::getDevicePermissionChecker(QString type)
{
    return _devicePermissionControllerMap.value(type, devicePermissionCheckerPtr());
}
