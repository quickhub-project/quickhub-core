/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef DEVICEHANDLELISTWRAPPER_H
#define DEVICEHANDLELISTWRAPPER_H

#include <QObject>
#include "../IList.h"
#include "Server/Devices/DeviceManager.h"

class DeviceHandleListWrapper : public IList
{

    Q_OBJECT

public:
    DeviceHandleListWrapper(QObject* parent = nullptr);
    virtual QVariantList getListData() const override;

private slots:
    void newHandle(QString uuid);
    void handleRemoved(QString uuid);
    void deviceStateChangedSlot(QString uuid, IDevice::DeviceState state);
    void deviceDescriptionChangedSlot(QString uuid, QString description);
    void newMapping(QString uuid, QString mapping);

private:
    void addHandle(deviceHandlePtr handle);
    QVariant toVariant(deviceHandlePtr handle);
    QMap<QString, QVariant> _devices;


    int getIndex(QString uuid) const;
};

#endif // DEVICEHANDLELISTWRAPPER_H
