/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "IDevice.h"

QString IDevice::shortId() const
{
    return "";
}

int IDevice::getFirmwareVersion() const
{
    return -1;
}

QVariantList IDevice::getSkills() const
{
    return QVariantList();
}

IDevice::DeviceError IDevice::initDevice(QVariantMap properties)
{
    QMapIterator<QString, QVariant> it(properties);
    while(it.hasNext())
    {
        DeviceError returnError = setDeviceProperty(it.key(), it.value());
        if(returnError != IDevice::NO_ERROR)
            return returnError;
    }
    return IDevice::NO_ERROR;
}

IDevice::DeviceError IDevice::startFirmwareUpdate(QVariant args)
{
    Q_UNUSED(args);
    return IDevice::FUNCTION_NOT_EXIST;
}
