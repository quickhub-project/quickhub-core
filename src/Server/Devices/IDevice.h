/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


/*!
    \class IDevice
    \brief This abstract class defines a generic API for (IoT-) Devices.
    \ingroup devices

    Derive this class to bring new kinds of devices into the system. After instanciating a derived class, you need to register
    the device to the DeviceManager. From that point on, the device will be listed in the device overview and can be mapped to a DeviceHandle.

    \note This class should NOT be used by external plugins to communicate with devices. Use DeviceHandle for this purpose! Don't forget to emit
    the appropriate change signals, when the internal state or properties have changed.

    \sa DeviceManager
*/

#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include "qhcore_global.h"
#include "Server/Authentication/IIdentitiy.h"

class COREPLUGINSHARED_EXPORT IDevice : public QObject, public IIdentity
{
    Q_OBJECT
    Q_PROPERTY(QString type     READ type   NOTIFY typeChanged)
    Q_PROPERTY(QString uuid     READ uuid   NOTIFY uuidChanged)


public:
    enum DeviceError
    {
        NO_ERROR = 0, /*! All okay, life is good */
        FUNCTION_NOT_EXIST = -1, /*! The function does not exist */
        DEVICE_NOT_AVAILABLE = -2, /*! The requested device does not exist */
        PROPERTY_NOT_EXISTS = -3, /*! The requested property does not exist on this device */
        PERMISSION_DENIED = -4 /*! You don't have the required permissions */
    };

    enum DeviceState
    {
        ONLINE, /*! The device is online and ready */
        OFFLINE, /*! The device is offline and out of service */
        SLEEPING, /*! The device is in deep sleep any may wake up periodically to check for updates*/
        STANDBY, /*! The device is stand by*/
        UPDATING, /*! The device is currently performing an Udpate */
        BUSY /*! The device is busy and cannot process your request */
    };

    explicit IDevice(QObject *parent = nullptr) : QObject(parent){}
    virtual ~IDevice(){}

    /*!
        \fn virtual QString uuid() const = 0
        This function must return the unique uuid of the device. In many cases this is the MAC-ID.
    */
    virtual QString     uuid()      const = 0;

    /*!
        \fn virtual QString type() const = 0
        This function must return the device type of device. The device type must be identical for all
        devices of this type. Try to find a term that is as meaningful and descriptive as possible.
        It may be advisable to include the version number of the hardware.
    */
    virtual QString     type()      const = 0;

    /*!
      \fn virtual QString shortId() const = 0
        This function can be overwritten to return a shortID. The short ID is intended to facilitate
        the provsioning of the device and consists of significantly fewer characters than the UUID.
        I recommend a four-digit ShortID consisting of the characters A-Z and 0-9. This Short ID can also
        be randomly generated if the probability is very low that the same ShortID is determined for two devices.
    */
    virtual QString     shortId()   const;

    /*!
        \fn virtual QVariantList getFunctions() const = 0
        This function must return all RPCs of a device. The return value consists of a QVariantList,
        which is actually a QList<QVariantMap>.  Each RPC is represented by a QVariantMap, which must have at least one "name" field containing the function name.

        example:
        <code>

            QVariantList func_list;
            QVariantMap func;
            func["name"] = "Im_a_function";
            func_list << func;
            return func_list;
       </code>

        QuickHub recognizes a property as writable if there is a corresponding setter RPC.
        The name of the RPC must be chosen according to the following convention:

        set<PropertyName>

        where PropertyName must start with an uppercase letter. If there is a property "temperture"
        the corresponding setter must be named "setTemperatur". If a property setter is called,
        the argument consists of a QVariantMap, where the new target value of the property is located
        in the field "val".

        After each call of a property-setter function a changed signal for the corresponding property must be emitted - even if the
        value hasn't really changed for some reasons - after receipt (from the device side). This allows QuickHub to recognize that the call has been successfully
        received by the device. If not, the property remains in the dirty state on the server side.
    */
    virtual QVariantList getFunctions() const = 0;
    virtual QVariantList getSkills() const;

    /*!
        \fn virtual QVariantMap getProperties() const = 0
        This function must return all properties of the device. As with RPCs,
        the return type is a QList<QVariantMap>, where each property is defined by a QVariantMap.
        The key is the name of the property and the value is the value of the property.
        \note See getFunctions() documentation to learn how to implement writable properties.
        \sa QVariantList getFunctions() const, DeviceError setDeviceProperty(QString property, QVariant value)
    */
    virtual QVariantMap getProperties() const = 0;


    /*!
        \fn DeviceError triggerFunction(QString name, QVariant parameters = QVariant()) = 0
        Calls the specified function with the given parameters. This function must be implemented.
        \note Make sure that corresponding error codes (NO_ERROR, FUNCTION_NOT_EXISTS and DEVICE_NOT_AVAILABLE) are returned correctly.
    */
    virtual IDevice::DeviceError triggerFunction(QString name, QVariant parameters = QVariant(), QString cbID = "") = 0;

    /*!
        \fn DeviceError setDeviceProperty(QString name, QVariant parameters = QVariant()) = 0
        Calls the property setter when there is one available. See documentation for "getFunctions" to lear more about
        writable properties.

        \note Make sure that corresponding error codes (NO_ERROR, FUNCTION_NOT_EXISTS and DEVICE_NOT_AVAILABLE) are returned correctly.
    */
    virtual DeviceError setDeviceProperty(QString property, QVariant value) = 0;


    /*!
        \fn DeviceError initDevice(QVariantMap properties)
        This function is called by the corresponding DeviceHandle if the set-values of properties
        have changed while the device was offline.
        Make sure to deliver the updates to the corresponding device.
    */
    virtual DeviceError initDevice(QVariantMap properties);

    /*!
        Return the device state.
    */
    virtual DeviceState getDeviceState() = 0;

    /*!
      This function can be overridden if your device supports the persistence of AuthenticationKeys.
      The AuthenticationKey is a secret which is generated during the first provisioning. It must be sent
      with every registration process.
    */
    virtual bool        enableAuthentificationKey(){return false;}

    /*!
        Make sure to persist this key in your non volotile device memory.
        \sa enableAuthentificationKey
    */
    virtual void        setAuthentificationKey(quint32 key){Q_UNUSED(key)}

    /*!
        Return the Authentification key you saved in setAuthentificationKey
        \sa enableAuthentificationKey
    */
    virtual quint32     getAuthentificationKey(){return 0;}

    /*!
       This function can be implemented, if your device supports OTA Updates.
       When this function is called by the DeviceService, a QVariantMap with the field
       "val" is provided, which contains the download URL for the firmware update
       \sa getFirmwareVersion()
    */

    virtual DeviceError startFirmwareUpdate(QVariant args);

    /*!
       This function can be implemented, if your device supports OTA Updates.
       The firmware versioning always consists of a major and a minor release. For example 7.12.
       From this an int is generated via <code>major * 1000 + minor</code>, which can be used for a
       direct int-comparison.
       \sa startFirmwareUpdate()
    */
    virtual int         getFirmwareVersion() const;

    /*!
      Returns the permissions requested by the device itself. This does not necessarily
      correspond to the permissions that the unit is actually entitled to after provisioning.
      Which permissions the device will actually have later is decided during the provisioning process.
    */
   virtual QMap<QString, bool> getRequestedPermissions() const = 0;


    virtual bool setToken(QString token) {Q_UNUSED(token); return false;};
    virtual bool isAuthorizedTo(QString permission) override;
    QMap<QString, bool> getGrantedPermissions() const;
    void setGrantedPermissions(const QMap<QString, bool> &grantedPermissions);

signals:
    // emit this signal, when you know that your device was propably offline
    // e.g. after connection loss
    void forcePropertySync();
    void propertyChanged(QString uuid, QString property, QVariant value);
    void deviceStateChanged(QString uuid, DeviceState state);
    void dataReceived(QString uuid, QString subject, QVariantMap data);
    void deregistered(QString uuid);
    void typeChanged();
    void uuidChanged();

private:
    QMap<QString, bool> _grantedPermissions;

};

#endif // IDEVICE_H
