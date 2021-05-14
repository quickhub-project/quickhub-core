/* Copyright (C) Friedemann Metzger - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Friedemann Metzger <friedemann.metzger@gmx.de>, 2017
*/
#include "DeviceQmlAdapter.h"

DeviceQmlAdapter::DeviceQmlAdapter(QObject *parent) : QQmlPropertyMap(this, parent)
{

}

QString DeviceQmlAdapter::mapping() const
{
    return _mapping;
}

void DeviceQmlAdapter::setMapping(const QString &mapping)
{
    _mapping = mapping;
    registerDevice(mapping);
    Q_EMIT deviceMappingChanged();
}

void DeviceQmlAdapter::call(QString function, QVariant parameters)
{
    _deviceHandle->triggerFunction(function, parameters);
}

QQmlListProperty<QObject> DeviceQmlAdapter::children()
{
    return QQmlListProperty<QObject>(this, _children);
}

bool DeviceQmlAdapter::registerDevice(QString deviceMapping)
{
    _deviceHandle = DeviceManager::instance()->getHandleByMapping(deviceMapping);
    if(!_deviceHandle.isNull())
    {
        connect(_deviceHandle.data(), &DeviceHandle::deviceStateChanged, this, &DeviceQmlAdapter::deviceStateChangedSlot);
        connect(_deviceHandle.data(), &DeviceHandle::propertyChanged, this, &DeviceQmlAdapter::propertyChangedSlot);
        connect(_deviceHandle.data(), &DeviceHandle::init, this, &DeviceQmlAdapter::initHandle);
        connect(_deviceHandle.data(), &DeviceHandle::dataReceived, this, &DeviceQmlAdapter::dataReceived);
        initHandle();
        return true;
    }

    return false;
}

bool DeviceQmlAdapter::online() const
{
    return _deviceHandle->getDeviceState() == IDevice::ONLINE;
}


QVariant DeviceQmlAdapter::updateValue(const QString &key, const QVariant &input)
{
    _deviceHandle->setDeviceProperty(key, input);
    return this->value(key);
}

void DeviceQmlAdapter::propertyChangedSlot(QString uuid, QString property, QVariant value, bool dirty)
{
    Q_UNUSED (uuid)
    Q_UNUSED (dirty)
    this->insert(property, value);
}

void DeviceQmlAdapter::deviceStateChangedSlot(QString uuid, IDevice::DeviceState state)
{
    Q_UNUSED (uuid)
    Q_UNUSED (state)
    Q_EMIT onlineChanged();
}

void DeviceQmlAdapter::initHandle()
{
    //*TODO*
    QVariantMap properties; /* = _deviceHandle->getConfirmedProperties();*/
    QMapIterator<QString, QVariant> it(properties);
    while(it.hasNext())
    {
        it.next();
        this->insert(it.key(), it.value());
    }
    Q_EMIT onlineChanged();
}
