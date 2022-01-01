/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef DEVICEHANDLEHANDLER_H
#define DEVICEHANDLEHANDLER_H

#include <QObject>
#include "SocketCore/IResourceHandler.h"
#include "Server/Devices/DeviceHandle.h"


class DeviceHandleHandler : public IResourceHandler
{
    Q_OBJECT

public:
    DeviceHandleHandler(QSharedPointer<DeviceHandle> deviceHandle, QObject* parent = nullptr);
    void initHandle(ISocket *handle) override;

private:
    virtual void handleMessage(QVariant message, ISocket* handle) override;
    void handleError(QString command, IDevice::DeviceError error, ISocket* socket);
    QSharedPointer<DeviceHandle> _deviceHandle;
    QVariantMap getDumpMessage();

    QMap<QString, ISocket* > _cbMap;

private slots:
    void registerProperty(DeviceProperty* property);
    void dataReceived(QString uuid, QString subject, QVariantMap data);
    void propertyChanged(QString uuid, QString property, QVariant value, bool dirty);
    void deviceStateChangedSlot(QString uuid, IDevice::DeviceState state);
    void deviceDescriptionChanged(QString uuid, QString description);
    void temporaryChanged(QString uuid, bool temporary);

    void metadataChanged(QString name, QString key, QVariant value);
    void confirmedChanged(QString name, qlonglong timestamp, bool accepted);
    void setValueChanged(QString name, QVariant setValue, bool dirty);
    void realValueChanged(QString name, QVariant realValue, bool dirty);
    void socketDisconnectedSlot();
   // void dirtyChanged(QString name, bool isDirty);

    void reinit();
};

#endif // DEVICEHANDLEHANDLER_H
