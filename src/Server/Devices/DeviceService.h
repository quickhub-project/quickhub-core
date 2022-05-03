/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include <QObject>
#include "Server/Services/IService.h"
#include "DeviceUpdateLogic.h"


/*!
    \class DeviceService
    \brief This class implements a service with function calls
    \ingroup devices
    \inherits IService

    This class provides a service with a set of functions to manage devices. Its derived from IService and allows
    to setup new devices remotely.

    Currently implemented service calls:

    hookWithShortID     - setup a new device via its shortID.
    unhookWithShortID   - removes a device mapping via its shortID
    checkForUpdates     - checks for available plugins for a given shortID
    startUpdate         - starts an update for a specific device with a download URL


    \sa DeviceHandleHandler DeviceManager
*/
class DeviceService : public IService
{
    Q_OBJECT

public:
    DeviceService(QObject* parent = nullptr);
    virtual QString         getServiceName() const override;
    virtual QStringList     getServiceCalls() const override;
    virtual bool            call(QString call, QString token, QString cbID, QVariant argument = QVariant()) override;
    QVariantMap             syncCalls(QString call, QString token, QVariant argument);

private:
    DeviceUpdateLogic* _updateLogic = nullptr;
};

#endif // DEVICESERVICE_H
