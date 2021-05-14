/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef DEVICEPROPERTY_H
#define DEVICEPROPERTY_H

#include <QObject>
#include "DeviceHandle.h"
#include <QReadWriteLock>

/*!
    \class DeviceProperty
    \brief This class represents a device property.
    \ingroup devices
    Each Device has a set of properties. Accordingly, the DeviceHandle has a DeviceProperty
    object for each property. The DeviceProperty object monitors the state of the individual
    properties and ensures that property changes are synchronized correctly, even if the Device is
    currently offline.
    This interface can be used if you want to read or change the property of a Device with a DeviceHandle.

    \sa DeviceHandle
*/
class DeviceProperty : public QObject
{
    Q_OBJECT
    friend class DeviceHandle;

public:
    /*!
        \fn  QVariant getValue() const
        This function returns the value that the property has or should have, if the property is dirty.
        If the property is dirty, i.e. not yet confirmed by the Device, the setValue is returned.
    */
    QVariant    getValue() const;

    /*!
        \fn  QVariant getRealValue() const
        This function always returns the last value confirmed by the Device.
        So the value, which the propertie on the device really has.
        If the Device has not yet confirmed a value change, this value differs from the set value.
    */
    QVariant    getRealValue() const;

    /*!
        \fn  QVariant getSetValue() const
        This function always returns the value the Device should have.
        So the value that was written by a client last. No matter which value
        was last confirmed by the Device.
    */
    QVariant    getSetValue() const;

    /*!
        \fn  QVariant getName() const
        This function returns the property name.
    */
    QString     getName() const;

    /*!
        \fn  void setValue(const QVariant &setValue);
        This function returns the unique name of this property.
    */
    void        setValue(const QVariant &setValue);

    /*!
        \fn  void setMetadata(const QVariant &setValue);
        This function allows to store additional parameteres that belongs to a property.
        This could be the unit string, a description string oder the display name of a particular property.
    */
    void        setMetadata(QString key, QVariant value);

    /*!
        \fn bool isDirty() const
        This function returns the dirty state of this property. A property is dirty whenever the timestamp of
        the setValues is newer than the timestamp of the last value change transmitted by the Device.

        The fact that a property is not dirty does not mean that the target value is confirmed by the Device.
        The concept is that after receiving a new property value, a device sends the new value
        back to the server as a property change. If the value returned from the device is the same as
        the target value requested by the client, the property has been confirmed. If the value is not the same,
        the Device has received the desired property change but has not confirmed it. In both cases the property
        is no longer dirty. It is only dirty if a property change request was not delivered to the device.
    */
    bool        isDirty() const;


    /*!
        \fn qlonglong confirmedTimestamp() const
        This function returns the time when the last value change was
        communicated from the Device to the server. From this you can see how current a value is.
    */
    qlonglong   confirmedTimestamp() const;

    /*!
        \fn QVariantMap toMap() const
        Returns a QVariantMap which contains all relevant data of this property instance.
        It's a helper function for persistence purposes.
    */
    QVariantMap toMap() const;

signals:
    // This signal is sent when a new value is transmitted by the Device
    void realValueChanged(QString name, QVariant getRealValue, bool dirty);

    // This signal is sent when a client has transmitted a new target value to the Device.
    // (Regardless of whether the Device is currently online or not)
    void setValueChanged(QString name, QVariant getValue, bool dirty);

    // This signal is sent, when the dirty status of this property has changed
    void dirtyChanged(QString name, bool isDirty);

    // This signal is sent, when the device has received a value change request.
    // Accepted is true, if the previously desired target value was accepted
    void confirmed(QString name, qlonglong timestamp, bool accepted);

    // Will be sent when the timestamp has changed
    void confirmedTimestampChanged(QString name, qlonglong timestamp);

    // Will be sent when the metadata has changed
    void metadataChanged(QString name, QString key, QVariant value);

private:
    void setRealValue(const QVariant &getRealValue, bool keepDirtyFlag = false);
    void setDirty(bool isDirty, bool accepted);
    explicit DeviceProperty(QString name, DeviceHandle *parent, QVariantMap metadata = QVariantMap());
    ~DeviceProperty();

private:
    QString         _name;
    QVariantMap     _metadata;
    QVariant        _realValue;
    QVariant        _setValue;
    bool            _dirty;
    qlonglong       _confirmedTimestamp;
    mutable QReadWriteLock  _mutex;



public slots:
};

#endif // DEVICEPROPERTY_H
