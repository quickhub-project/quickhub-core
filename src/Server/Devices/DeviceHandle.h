/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


/*!
    \class DeviceHandle
    \brief This class provides the API you should use, when you want to communicate directly with a device from your code.
    \ingroup devices

    This class is a kind of digital twin of a device.
    During provisioning, a Device Handle is created for each Device.
    The handle serves as contact point for all plugins and external
    interfaces. It tracks the state of the device and the state of
    the individual properties.

    \note Use this class to communicate with- or get infos from a specific device.

    \sa DeviceHandleHandler DeviceManager
*/

#ifndef DEVICEHANDLE_H
#define DEVICEHANDLE_H

#include <QObject>
#include "../Devices/IDevice.h"
#include <QSharedPointer>
#include "../Resources/ResourceManager/IResource.h"
#include "QReadWriteLock"

class DeviceProperty;
class IDevicePermissionChecker;
class DeviceHandle : public IResource
{
    Q_OBJECT

public:    
    ~DeviceHandle();
    explicit DeviceHandle(QString uuid, QString path, QObject *parent = nullptr);
    explicit DeviceHandle(QString path, QObject *parent = nullptr);

    /*!
        \fn void DeviceHandle::setDescription(QString description, QString token = "")
        Sets the description string of this device. The description string is a human readable string which is
        more convenient then the internal device resource address. The description string has no technical function and
        can be changed without any side effects.
        \sa DeviceHandle::getDescription()
    */
    void                            setDescription(QString description, QString token = "");

    /*!
        \fn void DeviceHandle::getDescription()
        Returns the description string of this device. The description string is a human readable string which is
        more convenient then the internal device resource address.
        \sa DeviceHandle::setDescription(QString description, QString token = "")
    */
    QString                         getDescription() const;

    /*!
        \fn void DeviceHandle::getDeviceState()
        Returns the current state of the device.

        \sa IDevice::DeviceState
    */
    IDevice::DeviceState            getDeviceState() const;

    /*!
        \fn void DeviceHandle::setDevice(IDevice* device)
        This function is mainly used by the DeviceManager. Either when devices reconnect or during provisioning.
        For external plugins there is normally no reason to use this function.
        \sa IDevice
    */
    bool                            setDevice(QSharedPointer<IDevice> device);

    /*!
        \fn void DeviceHandle::removeDevice()
        Destroys the connection between the handle and the Device UID.
    */
    bool                            removeDevice();

    /*!
        \fn void DeviceHandle::temporary()
        Returns true when the device is temporary. That means a client has instantiated
        a model for an device mapping which is actually not known in the system. After a Device is registered
        at this address (so it's not temporary anymore), this function returns false. Otherwise this handle will be deleted as soon
        as the client releases the model again.
    */
    bool                            temporary() const;

    /*!
        \fn void DeviceHandle::getFunctions()
        Returns a list with all registered RPC Functions for that device.
    */
    QVariantList                    getFunctions() const;

    /*!
        \fn void DeviceHandle::setDeviceProperty(QString property, QVariant value, QString token = "")
        Sets the value for the property. If there is a permission checker installed for this device, the
        implementation will check whether the appropriate user has permission to modify the device.

        \sa IDevicePermissionChecker
    */
    IDevice::DeviceError            setDeviceProperty(QString property, QVariant value, QString token = "");

    /*!
        \fn void DeviceHandle::type()
        Returns the device type string.
    */
    QString                         type() const;

    /*!
        \fn void DeviceHandle::uuid()
        Returns the device uuid string.
    */
    QString                         uuid() const;

    /*!
        \fn void DeviceHandle::shortUid()
        Returns the short ID. The short ID is in most cases a randomly generated ID (kind of alias), which can be used for device provisioning.
        It's intention is to simplify the provisioning process because the short ID is easier to remember then a long device uuid.
    */
    QString                         shortUid() const;

    /*!
        \fn void DeviceHandle::triggerFunction()
        Will invoke the appropriate RPC with the given parameteres.
        If there is a permission checker installed for this device, the
        implementation will check whether the appropriate user has permission to modify the device.
        \sa IDevice::DeviceError
    */
    IDevice::DeviceError triggerFunction(QString name, QVariant parameters, QString token = "", QString cbID ="");

    /*!
        \fn void DeviceHandle::propertyObjects()
        Returns a list with all property objects.

        \sa DeviceProperty
    */
    QMap<QString, DeviceProperty*>  propertyObjects();

    /*!
        \fn void DeviceHandle::properties()
        Returns a list with all properties as key-value pairs.
    */
    QVariantList                    properties() const;

    /*!
        \fn void DeviceHandle::property(QString name)
        Returns a pointer to the DeviceProperty Object instance for the given property name.
        Returns a nullptr, when the property doesn't exist.
    */
    DeviceProperty*                 property(QString name);

    /*!
        \fn DeviceHandle::getPropertyValue(QString name);
        Returns the value for a given property. If the property doesn't exist, the QVariant will be invalid.
        \sa QVariant::isValid()
    */
    QVariant                        getPropertyValue(QString name) const;

    /*!
        \fn DeviceHandle::getAuthentificationKey() const
        Returns the authentification key that is used to uniquely identify the Device.
        The authentification key is generated  by the DeviceManager during provisioning and must be sent by
        the Device each time it logs on to the server.
        \sa DeviceHandle::setAuthentificationKey()
    */
    quint32                         getAuthentificationKey() const;

    /*!
        \fn DeviceHandle::setAuthentificationKey(const quint32 &securekey)
        sets the authentification key that is used to uniquely identify the Device.
        The authentification key is generated by the DeviceManager during provisioning and must be sent by
        the Device each time it logs on to the server.
        \sa DeviceHandle::getAuthentificationKey()
    */
    void                            setAuthentificationKey(const quint32 &securekey);

    /*!
        \fn DeviceHandle::getEnableSecureCheck() const
        Returns true, when the attached device supports authentication with a authentification key.
        \sa DeviceHandle::getAuthentificationKey(), DeviceHandle::setAuthentificationKey()
    */
    bool                            getEnableSecureCheck() const;

    /*!
        \fn DeviceHandle::startFirmwareUpdate()
        Will start an update process if the function is implemented by the corresponding Device.
        \sa DeviceHandle::getFirmwareVersion()
    */
    IDevice::DeviceError            startFirmwareUpdate(QVariant args);
    int                             getFirmwareVersion();

    QVariantMap                     getPermissions();
    void                            setPermissions(const QMap<QString, bool> &permissions);

private:
    /*!
        These functions are overwritten from IResource and are for persistance purposes.
    */
    virtual const QVariantMap       getData() override;
    virtual const QString           getResourceType() const override;
    virtual qint64                  lastAccess() const override;


    /*!
        \fn void DeviceHandle::setUuid(QString uuid)
        Assigns a new device-uuid to this handle. This function is used internally when a remapping occurs.
    */
    void                            setUuid(QString uuid);

    DeviceProperty*                 createDevicePropertyObject(QString name, DeviceHandle *parent, QVariantMap metadata = QVariantMap());
    void                            loadLastData();
    void                            registerPropertyObject(QString name,DeviceProperty* prop);
    QMap<QString, DeviceProperty*>  _properties;
    QMap<QString, bool>             _permissions;
    bool                            _temporary = false;
    bool                            _initialized = false;
    QStringList                     _mappings;
    qint64                          _lastAccess = 0;
    qint64                          _lastOnline = 0;
    QString                         _uuid;
    QString                         _shortID;
    QVariantList                    _functions;
    QSharedPointer<IDevice>         _device;
    IDevice::DeviceState            _deviceSate = IDevice::OFFLINE;
    QString                         _type;
    QString                         _description;
    quint32                         _authentificationKey = 0;
    QString                         _token;
    int                             _firmwareVersion;
    bool                            _enableSecureCheck = false;
    mutable QReadWriteLock          _lock;
  QSharedPointer<IDevicePermissionChecker>      _permissionChecker;

signals:
    void deviceStateChanged(QString uuid, IDevice::DeviceState state);
    void newPropertyObject(DeviceProperty* property);
    void dataReceived(QString uuid, QString subject, QVariantMap data);
    void propertyChanged(QString uuid, QString property, QVariant value, bool dirty);
    void uuidChanged(QString uuid);
    void descriptionChanged(QString uuid, QString description);
    void temporaryChanged(QString uuid, bool temporary);
    void init();

private slots:
    void syncDevice();
    void sendPropertyToDevice(QString name, QVariant value);
    void deviceStateChangedSlot(QString uuid, IDevice::DeviceState state);
    void deviceDeregistered(QString uuid);
    void deviceRegistered(QString uuid);
    void propertyChangedSlot(QString uuid, QString property, QVariant value);
};

#endif // DEVICEHANDLE_H
