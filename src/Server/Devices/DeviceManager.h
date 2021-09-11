/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


/*!
    \class DeviceManager
    \brief This class is responsible for managing (registering and deregistering) devices and its handles.
    \ingroup devices

    Use the API of this class when you ..
    - if you need a pointer to the handle of a certain device.
    - when you want to provision a new Device
    - if you want to remove an existing device mapping

    I recommend to read https://docs.2log.io/doku.php?id=quickhub_devices to get the big picture.
    \sa DeviceHandle
*/
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QFile>

#include "../Defines/ErrDef.h"
#include "IDevice.h"
#include "DeviceHandle.h"
#include <QSharedPointer>
#include <QWeakPointer>

typedef QSharedPointer<IDevice> iDevicePtr;
typedef QSharedPointer<DeviceHandle> deviceHandlePtr;
typedef QWeakPointer<DeviceHandle> weakDeviceHandlePtr;

class COREPLUGINSHARED_EXPORT DeviceManager : public QObject
{
    Q_OBJECT

public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager();

    /*!
        \fn bool DeviceManager::registerDevice(iDevicePtr device)
        This function needs to be called every time a device logs on.
        Regardless of whether it is a hardware device or a virtual software device.
        If the Device is already provisioned, the corresponding handle is assigned
        the appropriate IDevice instance.
    */
    bool registerDevice(iDevicePtr device);

    /*!
        \fn bool DeviceManager::exists(QString uuid)
        Returns true if there is a registered device with this uuid
    */
    bool exists(QString uuid);

    /*!
        \fn Err::CloudError DeviceManager::setDeviceMapping(QString token, QString mapping, QString uuid, bool force = true);
        Use this function to setup a new device. This fuction will register a new address (mapping) for the device with the given uuid.
        Only users with the permission "MANAGE_DEVICES" are allowed to do this.

        If yout want to remove the device, you just need to provide an empty uuid string for an registered device address.

        An error will be returned if the device or the address is already in use.
        If force is set to true, all warnings will be ignored.
    */
    Err::CloudError setDeviceMapping(QString token, QString mapping, QString uuid, bool force = true);

    /*!
        \fn Err::CloudError DeviceManager::setDeviceMappingByShortId(QString token, QString mapping, QString shortID, bool force = true);
        If the Device has logged in with a shortID, this function can be used to create a new mapping.
        Furthermore the function behaves exactly like setDeviceMapping()
        \sa DeviceManager::setDeviceMapping(QString token, QString mapping, QString uuid, bool force = true)
    */
    Err::CloudError setDeviceMappingByShortId(QString token, QString mapping, QString shortID, bool force = true);

    /*!
        \fn Err::CloudError DeviceManager::prepareDeviceMapping(QString token, QString mapping, QString uuid)
        IOffers the possibility to register a Device which is not (yet) connected to the server. This function is used for
        provisioning if the app already knows the UID of the Device but the Device is not yet connected.
        \sa DeviceManager::setDeviceMapping(QString token, QString mapping, QString uuid, bool force = true)
    */
    Err::CloudError prepareDeviceMapping(QString token, QString mapping, QString uuid);

    /*!
        \fn void DeviceManager::init(QString storagePath)
        the init function is called during the initialization of the Core Plugin All mappings and device
        information are stored in the provided storagePath
    */
    void init(QString storagePath);

    /*!
        \fn QMap<QString, QString>  getMappings() const
        Returns a list with all registered mappings from the internal address to the device uuid.
        The key specifies the address string, value the device id string.
    */
    QMap<QString, QString>    getMappings() const;

    /*!
        \fn deviceHandlePtr getHandle(QString uuid) const
        Returns a handle to the device with the given uuid. The hanle will be null if there is no device with this uuid.
    */
    deviceHandlePtr           getHandle(QString uuid) const;

    /*!
        \fn deviceHandlePtr getHandleByMapping(QString mapping)
        Returns a handle to the device with the given mapping. The hanle will be null if there is no device with this uuid.
    */
    deviceHandlePtr           getHandleByMapping(QString mapping);

    /*!
        \fn QList<deviceHandlePtr>  getHandles()
        Returns a list with all registered device handles.
    */
    QList<deviceHandlePtr>    getHandles();

    /*!
        \fn QStringList getDevices() const
        Returns a list with all registered device uuids.
    */
    QStringList               getDevices() const;

    /*!
        \fn QStringList getDeviceByUuid() const
        Returns a pointer to the IDevice instance.
        \note You should NOT use IDevice* instances to work and communicate with devices -
        Use DeviceHandles, accessible via getHandle(..) or getHandlyByMapping(..) instead!
    */
    iDevicePtr getDeviceByUuid(QString uuid) const;

    /*!
        \fn QString getDeviceByMapping() const
        Returns the device UUID via its registered address.
    */
    QString                   getDeviceByMapping(QString mapping) const;

    /*!
        \fn QString getTypeForUuid(QString uuid) const
        Returns the device type for a given device UUID.
    */
    QString                   getTypeForUuid(QString uuid) const;
    QString                   getUuidForShortId(QString shortID) const;


    static DeviceManager* instance();

private:
    QMap<QString, deviceHandlePtr>      _handles; // uuid -> handle
    QMap<QString, weakDeviceHandlePtr>  _handleByMappings; // mapping -> handle
    QMap<QString, iDevicePtr>           _deviceMap; // uuid -> device
    QMap<QString, QString>              _deviceMappings; // mapping -> uuid
    QMap<QString, QString>              _shortIDtoUid; // shortID -> uuid
    QMap<QString, QString>              _preparedHooks; // hooks that will be set when device comes online
    QString                             _storagePath;

private:
    Err::CloudError unhook(QString mapping);
    Err::CloudError hook(QString mapping, QString uuid, bool force = true);


    deviceHandlePtr addDeviceHandle(QString uuid);
    void saveMappings();
    void loadMappings();
    void loadHandles();

private slots:
    void deregisterDevice(QString uuid);

signals:
    // This signal is sent when a new Device logs in.
    void deviceRegistered(QString uuid);

    // This signal is sent when a new Device logs out.
    void deviceDeregistered(QString uuid);

    // This signal is sent when a new device gets registered.
    void newDeviceMapping(QString uuid, QString mapping);

    // This signal is sent when a new device is assigned to an address
    void newDeviceHandle(QString uuid);

    // This signal is sent when a device is disconnected from its address
    void deviceHandleRemoved(QString uuid);

    // This signal is sent when a device is disconnected from its address
    void deviceMappingRemoved(QString uuid, QString mapping);


public slots:
};

#endif // DEVICEMANAGER_H
