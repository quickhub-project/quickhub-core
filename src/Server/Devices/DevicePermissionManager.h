/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef DEVICEPERMISSIONMANAGER_H
#define DEVICEPERMISSIONMANAGER_H

#include <QObject>
#include "IDevicePermissionController.h"
#include <QMap>


/*!
    \class DevicePermissionManager
    \brief This class implements a lookup table from device type to the according permission checker
    \ingroup devices
    DevicePermissionChecker defines the access rights of users to specific devices or device types.
    Whenever a user tries to access a device, the system checks if a DevicePermissionChecker has been
    defined for this device type. Within DevicePermissionChecker you define which users are allowed to
    read or write the device parameters.

    \sa DevicePermissionChecker, DeviceManager
*/


class DevicePermissionManager : public QObject
{
    Q_OBJECT

public:
    explicit DevicePermissionManager(QObject *parent = nullptr);

    static DevicePermissionManager* instance();

    /*!
        \fn bool registerPermissionChecker(devicePermissionCheckerPtr checker)
        With this function a DevicePermissionChecker can be stored for a specific Device type.
        Returns false if a DevicePermissionChecker has already been registered for this device type
    */
    bool                            registerPermissionChecker(devicePermissionCheckerPtr checker);

    /*!
        \fn devicePermissionCheckerPtr getDevicePermissionChecker(QString type)
        returns a DevicePermissionChecker for the given DeviceType.
    */
    devicePermissionCheckerPtr      getDevicePermissionChecker(QString type);

private:
    QMap<QString, devicePermissionCheckerPtr> _devicePermissionControllerMap;

signals:

};

#endif // DEVICEPERMISSIONMANAGER_H
