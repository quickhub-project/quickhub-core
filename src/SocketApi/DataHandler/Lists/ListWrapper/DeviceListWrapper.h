/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef DEVICELISTWRAPPER_H
#define DEVICELISTWRAPPER_H

#include <QObject>
#include "../IList.h"
#include "Server/Devices/IDevice.h"

class DeviceListWrapper : public IList
{
    Q_OBJECT

public:
    DeviceListWrapper(QObject* parent = nullptr);
    virtual QVariantList getListData() const override;

private:
    QVariantList _list;
    QVariantMap toMap(QSharedPointer<IDevice> device) const;
    bool addDevice(QString uuid);
    int getIndex(QString uuid) const;
    void handleMessage(QVariant msg, ISocket *handle) override;

private slots:
    void newDevice(QString uuid);
    void deviceRemoved(QString uuid);
    void newMapping(QString uuid, QString mapping);
    void mappingRemoved(QString uuid, QString mapping);
    void deviceStateChanged(QString uuid, IDevice::DeviceState state);
};

#endif // DEVICELISTWRAPPER_H
