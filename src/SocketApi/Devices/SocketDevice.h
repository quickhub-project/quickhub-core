/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


/*!
    \class WebsocketDevice
    \brief This class implements a protocol to communicate with devices that are using connection based communication.
    \ingroup IoT Devices

    One counterpart of this implementation is the DeviceNode library written for ESP8266 microcontrollers. This class implements the
    abstract interface definition of IDevice. SocketDeviceHandler will create an instance for each connected device and hand off the pointer
    to the DeviceManager.

    \sa SocketDeviceHandler, IDevice, DeviceManager
*/

#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QVariant>
#include "Server/Devices/IDevice.h"
#include "Connection/VirtualConnection.h"
#include <QTimer>

class SocketDevice : public IDevice
{
    Q_OBJECT
    Q_PROPERTY(QString macID READ macID NOTIFY macIDChanged)

public:
    explicit SocketDevice(QObject *parent = nullptr);
    ~SocketDevice(){qDebug()<<"RIP Device";}
    void init(QVariantMap data, ISocket* handle);
    QString macID() const;
    QString uuid() const override;
    QString shortId() const override;
    QString type() const override;
    QVariantMap getProperties() const override;
    QVariantList getFunctions() const override;
    IDevice::DeviceError triggerFunction(QString name, QVariant parameters, QString cbID ="") override;
    IDevice::DeviceError setDeviceProperty(QString property, QVariant value) override;
    IDevice::DeviceError initDevice(QVariantMap properties) override;
    IDevice::DeviceState getDeviceState() override;
    void setAuthentificationKey(quint32 key) override;
    quint32 getAuthentificationKey() override;
    bool enableAuthentificationKey() override;
    DeviceError startFirmwareUpdate(QVariant args) override;
    virtual int getFirmwareVersion() const override;
    bool setToken(QString token) override;

    //IIDentity
    virtual QString         identityID() const override ;
    QMap<QString, bool>     getRequestedPermissions() const override;

private:
    QString getPropertySetterFunc(QString propertyName) const;
    QTimer                      _ackTimer;
    ISocket*                    _deviceConnection = nullptr;
    QVariantMap                 _properties;
    QVariantList                _functions;
    QMap<QString, QVariantMap>  _functionParameters;
    QMap<QString, bool>         _permissions;
    QString                     _token;

    QString             _macID;
    QString             _shortID;
    QString             _type;
    QString             _uuid;
    quint32             _authKey;
    quint16             _lastCbID = 0;

private slots:
    void messageReceived(QVariant msg);
    void connectionDisconnected();
    void sendAck();

signals:
    void macIDChanged();


public slots:
};

#endif // DEVICE_H
