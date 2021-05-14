/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "DeviceHandle.h"
#include "DeviceManager.h"
#include "DeviceProperty.h"
#include <QtDebug>
#include <QDateTime>
#include "DevicePermissionManager.h"
#include <QWriteLocker>
#include <QReadLocker>
#include "../Authentication/AuthentificationService.h"
#include "../Authentication/IIdentitiy.h"

DeviceHandle::DeviceHandle(QString uuid, QString path, QObject *parent) : IResource(path, parent),
    _uuid(uuid),
    _lock(QReadWriteLock::Recursive)
{
    connect(DeviceManager::instance(), &DeviceManager::deviceRegistered, this, &DeviceHandle::deviceRegistered);
    connect(DeviceManager::instance(), &DeviceManager::deviceDeregistered, this, &DeviceHandle::deviceDeregistered);
    IDevice* device = DeviceManager::instance()->getDeviceByUuid(_uuid);
    if(device) // will be only the case in "set mapping" process
        setDevice(device);
    else
        loadLastData();
}

DeviceHandle::DeviceHandle(QString path, QObject *parent): IResource(path, parent),
    _temporary(true),
    _lock()
{
    connect(DeviceManager::instance(), &DeviceManager::deviceRegistered, this, &DeviceHandle::deviceRegistered);
    connect(DeviceManager::instance(), &DeviceManager::deviceDeregistered, this, &DeviceHandle::deviceDeregistered);
}

DeviceHandle::~DeviceHandle()
{
    qDebug()<< "Device-Handle Destroyed for: " + getResourcePath() + ( !_uuid.isEmpty() ? "/"+ _uuid : "") ;
    QReadLocker locker(&_lock);
    if(!_temporary)
    {
       locker.unlock();
       save();
    }
}

const QVariantMap DeviceHandle::getData()
{
    QVariantMap data;
    QVariantMap properties;

    _lock.lockForRead();
    QMapIterator<QString, DeviceProperty*> propIt(_properties);
    while(propIt.hasNext())
    {
        propIt.next();
        properties.insert(propIt.key(), propIt.value()->toMap());
    }

    data["properties"] = properties;
    data["functions"] = _functions;
    data["type"] = _type;
    data["lastOnline"] = _lastOnline;
    data["description"] = _description;
    data["authkey"] = _authentificationKey;
    data["enableauthkey"] = _enableSecureCheck;
    data["shortID"] = _shortID;
    _lock.unlock();
    return data;
}

void DeviceHandle::setUuid(QString uuid)
{
    if(uuid == _uuid)
        return;

    _lock.lockForWrite();
    _initialized = false;
    _uuid = uuid;
    _lock.unlock();
    Q_EMIT uuidChanged(uuid);
}

void DeviceHandle::setDescription(QString description, QString token)
{
    Q_UNUSED(token)
    _lock.lockForWrite();
    _description = description;
    _lock.unlock();
    save();
    Q_EMIT descriptionChanged(_uuid, description);
}

qint64 DeviceHandle::lastAccess() const
{
    QReadLocker locker(&_lock);
    return _lastAccess;
}

const QString DeviceHandle::getResourceType() const
{
    return "device";
}

IDevice::DeviceState DeviceHandle::getDeviceState() const
{
    QReadLocker locker(&_lock);
    return _deviceSate;
}

QString DeviceHandle::getDescription() const
{
    QReadLocker locker(&_lock);
    return _description;
}

bool DeviceHandle::setDevice(IDevice *device)
{
    if(device == nullptr)
        return false;

    bool hasDevice = false;
    QReadLocker locker(&_lock);
    if(_device != nullptr)
        hasDevice = true;

    locker.unlock();
    if(hasDevice)
        removeDevice();

    setUuid(device->uuid());
    connect(device, &IDevice::propertyChanged, this, &DeviceHandle::propertyChangedSlot);
    connect(device, &IDevice::dataReceived, this, &DeviceHandle::dataReceived);
    connect(device, &IDevice::deviceStateChanged, this, &DeviceHandle::deviceStateChangedSlot);
    connect(device, &IDevice::forcePropertySync, this, &DeviceHandle::syncDevice);
    //_deviceOnline = true;

    _lock.lockForWrite();
    _device = device;
    _deviceSate = device->getDeviceState();
    _firmwareVersion = device->getFirmwareVersion();
    _temporary = false;
    _lock.unlock();
    Q_EMIT deviceStateChanged(_uuid, _deviceSate);
    Q_EMIT temporaryChanged(_uuid, _temporary);
    syncDevice();

    bool emitInit = false;
    _lock.lockForWrite();
    if(!_initialized)
    {
        _initialized = true;
        emitInit = true;
    }
    _lock.unlock();

    if(emitInit)
        Q_EMIT init();

    save();
    return true;
}

bool DeviceHandle::removeDevice()
{
    QReadLocker locker(&_lock);
    if(_device == nullptr)
        return false;
    locker.unlock();

    setUuid("");
    disconnect(_device, &IDevice::propertyChanged, this, &DeviceHandle::propertyChangedSlot);
    disconnect(_device, &IDevice::dataReceived, this, &DeviceHandle::dataReceived);
    disconnect(_device, &IDevice::deviceStateChanged, this, &DeviceHandle::deviceStateChangedSlot);
    disconnect(_device, &IDevice::forcePropertySync, this, &DeviceHandle::syncDevice);
    _lock.lockForWrite();
    _deviceSate = IDevice::OFFLINE;
    _temporary = true;
    _device = nullptr;
    _lock.unlock();
    Q_EMIT deviceStateChanged(_uuid, _deviceSate);
    Q_EMIT temporaryChanged(_uuid,true);
    save();
    return true;
}

bool DeviceHandle::temporary() const
{
    QReadLocker locker(&_lock);
    return _temporary;
}


QVariantList DeviceHandle::getFunctions() const
{
    QReadLocker locker(&_lock);
    return _functions;
}

IDevice::DeviceError DeviceHandle::setDeviceProperty(QString property, QVariant value, QString token)
{
    IDevicePermissionChecker::PropertyPermission permission;
    _lock.lockForRead();
    if(!_permissionChecker.isNull())
        permission = _permissionChecker-> checkPropertyPermission(token, this, property);
    _lock.unlock();

    if(!permission.canWrite)
        return IDevice::PERMISSION_DENIED;

    _lock.lockForRead();
    DeviceProperty* prop = _properties.value(property, nullptr);
    _lock.unlock();

    if(!prop)
        return IDevice::PROPERTY_NOT_EXISTS;

    prop->setValue(value);
    return IDevice::NO_ERROR;
}

QString DeviceHandle::type() const
{
    QReadLocker locker(&_lock);
    return _type;
}

QString DeviceHandle::uuid() const
{
    QReadLocker locker(&_lock);
    return _uuid;
}

QString DeviceHandle::shortUid() const
{
    QReadLocker locker(&_lock);
    return _shortID;
}

IDevice::DeviceError DeviceHandle::triggerFunction(QString name, QVariant parameters, QString token, QString cbID)
{
    bool canCall = true;
    _lock.lockForRead();
    if(!_permissionChecker.isNull())
        canCall = _permissionChecker->checkRPCPermission(token, this, name);
    _lock.unlock();

    if(!canCall)
        return IDevice::PERMISSION_DENIED;

    QReadLocker locker(&_lock);
    if(_deviceSate != IDevice::ONLINE)
        return IDevice::DEVICE_NOT_AVAILABLE;
    QVariantMap paramMap = parameters.toMap();
    iIdentityPtr identity  = AuthenticationService::instance()->validateToken(token);
    if(!identity.isNull())
        paramMap["caller"] = identity.data()->identityID();

    return _device->triggerFunction(name, paramMap, cbID);
}

QMap<QString, DeviceProperty *> DeviceHandle::propertyObjects()
{
    QReadLocker locker(&_lock);
    return _properties;
}

QVariantList DeviceHandle::properties() const
{
    QVariantList properties;
    QReadLocker locker(&_lock);
    QMapIterator<QString, DeviceProperty*> propIt(_properties);
    while(propIt.hasNext())
    {
        properties <<  propIt.next().value()->toMap();
    }
    return properties;
}

DeviceProperty *DeviceHandle::property(QString name)
{
    QReadLocker locker(&_lock);
    return _properties.value(name, nullptr);
}

DeviceProperty* DeviceHandle::createDevicePropertyObject(QString name, DeviceHandle *parent, QVariantMap metadata)
{
    DeviceProperty* prop = new DeviceProperty(name, parent, metadata);
    connect(prop, &DeviceProperty::metadataChanged, this, &DeviceHandle::save);
    return prop;
}

QVariant DeviceHandle::getPropertyValue(QString name) const
{
    QReadLocker locker(&_lock);
    DeviceProperty* prop = _properties.value(name, nullptr);
    if(prop == nullptr)
        return QVariant(); //invalid

    return prop->getValue();
}

void DeviceHandle::syncDevice()
{
    QReadLocker locker(&_lock);
    if(_device == nullptr)
        return;

    QVariantMap newProperties = _device->getProperties();
    locker.unlock();

    QMapIterator<QString,QVariant> receivedPropertiesIt(newProperties);

    QVariantMap _unconfirmedProperties;
    while (receivedPropertiesIt.hasNext())
    {
        receivedPropertiesIt.next();
        QString key = receivedPropertiesIt.key();
        QVariant val = receivedPropertiesIt.value();
        _lock.lockForRead();
        DeviceProperty* property = _properties.value(key, nullptr);
        _lock.unlock();
        if(property)
        {
            // These values come directly from the sensor after reattach. If there are shadowed values,
            // then let the dirty flag as it is!
            property->setRealValue(val, true);
        }
        else
        {
             property = createDevicePropertyObject(key, this);// new DeviceProperty(key, this);
             property->setRealValue(val);
             registerPropertyObject(key, property);
        }

        _lock.lockForRead();
        QMapIterator<QString, DeviceProperty*> it = _properties;

        while(it.hasNext())
        {
            if(it.next().value()->isDirty())
            {
                _unconfirmedProperties.insert(it.key(), it.value()->getSetValue());
            }
        }
        _lock.unlock();
        Q_EMIT propertyChanged(_uuid, key, val, false);
    }
    _lock.lockForWrite();
    _functions = _device->getFunctions();
    _type = _device->type();
    _permissionChecker = DevicePermissionManager::instance()->getDevicePermissionChecker(_type);
    _shortID = _device->shortId();
    _device->initDevice(_unconfirmedProperties);
    _lock.unlock();
    save();
}

void DeviceHandle::loadLastData()
{
    QVariantMap data = load();
    if(data.isEmpty())
        return;

    _lock.lockForWrite();
    _type = data["type"].toString();
    _permissionChecker = DevicePermissionManager::instance()->getDevicePermissionChecker(_type);
    _functions = data["functions"].toList();
    _shortID = data["shortID"].toString();
    _description = data["description"].toString();
    _authentificationKey = data["authkey"].toUInt();
    _enableSecureCheck = data["enableauthkey"].toBool();
    _lastOnline = data["lastOnline"].toLongLong();
    _lock.unlock();

    QVariantMap properties = data["properties"].toMap();
    QMapIterator<QString, QVariant> propIt(properties);

    while (propIt.hasNext())
    {
        propIt.next();
        DeviceProperty* prop = createDevicePropertyObject(propIt.key(), this,  propIt.value().toMap());//new DeviceProperty(propIt.key(), this,  propIt.value().toMap());
        registerPropertyObject(propIt.key(), prop);
    }

}

void DeviceHandle::registerPropertyObject(QString name, DeviceProperty *prop)
{
    _lock.lockForWrite();
    _properties.insert(name, prop);
    _lock.unlock();
    connect(prop, &DeviceProperty::setValueChanged, this, &DeviceHandle::sendPropertyToDevice);
    Q_EMIT newPropertyObject(prop);
}

bool DeviceHandle::getEnableSecureCheck() const
{
    QReadLocker locker(&_lock);
    return _enableSecureCheck;
}

IDevice::DeviceError DeviceHandle::startFirmwareUpdate(QVariant args)
{
    if(_device == nullptr)
        return IDevice::DEVICE_NOT_AVAILABLE;

    return _device->startFirmwareUpdate(args);
}

int DeviceHandle::getFirmwareVersion()
{
    return _firmwareVersion;
}

quint32 DeviceHandle::getAuthentificationKey() const
{
    QReadLocker locker(&_lock);
    return _authentificationKey;
}

void DeviceHandle::setAuthentificationKey(const quint32 &securekey)
{
    _lock.lockForWrite();
    _authentificationKey = securekey;
    _enableSecureCheck = true;
    _lock.unlock();
    save();
}


void DeviceHandle::sendPropertyToDevice(QString name, QVariant value)
{
    DeviceProperty* senderProp = qobject_cast<DeviceProperty*>(sender());
    if(!senderProp)
        return;

    _lock.lockForRead();
    IDevice* device = _device;
    _lock.unlock();
    if(device != nullptr)
    {
        device->setDeviceProperty(name, value);
    }
    else
    {
        save();
    }

    Q_EMIT propertyChanged(_uuid, name, value, true);
}

void DeviceHandle::deviceStateChangedSlot(QString uuid, IDevice::DeviceState state)
{
    _lock.lockForWrite();
    _deviceSate = state;
    _lock.unlock();
    if(state == IDevice::ONLINE)
        syncDevice();

    Q_EMIT deviceStateChanged(uuid, state);
}

void DeviceHandle::deviceDeregistered(QString uuid)
{
    QReadLocker locker(&_lock);
    if(uuid != _uuid || _device == nullptr)
        return;

    disconnect(_device, &IDevice::propertyChanged, this, &DeviceHandle::propertyChangedSlot);
    disconnect(_device, &IDevice::dataReceived, this, &DeviceHandle::dataReceived);
    disconnect(_device, &IDevice::deviceStateChanged, this, &DeviceHandle::deviceStateChangedSlot);
    locker.unlock();
    save();

    _lock.lockForWrite();
    _lastOnline = QDateTime::currentMSecsSinceEpoch();
    _deviceSate = IDevice::OFFLINE;
    _device = nullptr;
    _lock.unlock();
    Q_EMIT deviceStateChanged(_uuid, IDevice::OFFLINE);
}

void DeviceHandle::deviceRegistered(QString uuid)
{
    QReadLocker locker(&_lock);
    if(_device)
        return;

    if(uuid == _uuid)
    {
        locker.unlock();

        IDevice* device = DeviceManager::instance()->getDeviceByUuid(uuid);
        if(!device)
            return;

        locker.relock();
        if(_enableSecureCheck)
        {
            if(_authentificationKey != device->getAuthentificationKey())
            {
                qWarning()<<"Wrong authentification key: "+_uuid+"! - Device rejected.";
                qDebug()<< "IS:     "+QString::number(device->getAuthentificationKey());
                qDebug()<< "SHOULD: "+QString::number(_authentificationKey);
                return;
            }
            else
            {
                qDebug()<<"Check succeeded: "<<device->getAuthentificationKey();
            }
        }
        locker.unlock();

        setDevice(device);
    }
}

void DeviceHandle::propertyChangedSlot(QString uuid, QString property, QVariant value)
{
    Q_UNUSED(uuid)
    _lock.lockForRead();
    DeviceProperty* prop = _properties.value(property, nullptr);
    _lock.unlock();

    if(prop)
    {
        prop->setRealValue(value);
    }
    else
    {
         prop = createDevicePropertyObject(property, this);// new DeviceProperty(property, this);
         prop->setRealValue(value);
         registerPropertyObject(property, prop);
    }

    Q_EMIT propertyChanged(_uuid, property, value, false);
}
