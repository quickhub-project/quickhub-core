/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IDEVICEPERMISSIONCONTROLLER_H
#define IDEVICEPERMISSIONCONTROLLER_H

#include <QObject>
#include <QSharedPointer>
#include "DeviceManager.h"

class DeviceHandle;
class IDevicePermissionChecker : public QObject
{
    Q_OBJECT

public:
    struct PropertyPermission
    {
        bool canRead = true;
        bool canWrite = true;
    };

    explicit IDevicePermissionChecker(QObject *parent = nullptr):QObject(parent){};
    ~IDevicePermissionChecker(){};

    virtual QStringList         getControlledDeviceTypes() = 0;
    virtual PropertyPermission  checkPropertyPermission(QString token, DeviceHandle* handle, QString property) = 0;
    virtual bool                checkRPCPermission(QString token, DeviceHandle* handle, QString rpc) = 0;
    virtual bool                checkSetupPermission(QString token, DeviceHandle* handle) = 0;

signals:

};

typedef QSharedPointer<IDevicePermissionChecker> devicePermissionCheckerPtr;

#endif // IDEVICEPERMISSIONCONTROLLER_H
