/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "DeviceHandle.h"
#include "DeviceManager.h"
#include "DevicePermissionManager.h"
#include "DeviceProperty.h"

#include "../Authentication/AuthentificationService.h"
#include "../Authentication/IIdentitiy.h"

#include <QDateTime>
#include <QLoggingCategory>
#include <QReadLocker>
#include <QWriteLocker>

// ----------------------------------------------------------------------------
// Logging category
// ----------------------------------------------------------------------------
Q_LOGGING_CATEGORY(lcDeviceHandle, "quickhub.device.handle")

// ----------------------------------------------------------------------------
// Ctors / Dtor
// ----------------------------------------------------------------------------
DeviceHandle::DeviceHandle(QString uuid, QString path, DeviceManager *dm)
    : IResource(path, nullptr),
      _uuid(std::move(uuid)),
      _lock(QReadWriteLock::Recursive),
      _deviceManager(dm)
{
    if (_deviceManager.isNull())
    {
        qCWarning(lcDeviceHandle) << "DeviceManager is null. Handle is inactive."
                                  << "path:" << getResourcePath()
                                  << "uuid:" << _uuid;
        return;
    }

    connect(dm, &DeviceManager::deviceRegistered, this, &DeviceHandle::deviceRegistered);
    connect(dm, &DeviceManager::deviceDeregistered, this, &DeviceHandle::deviceDeregistered);

           // This is only expected during "set mapping". In all other cases, the handle is created at startup.
    const iDevicePtr device = dm->getDeviceByUuid(_uuid);
    if (device)
    {
        setPermissions(device->getRequestedPermissions());
        setDevice(device);
    }
    else
    {
        qCInfo(lcDeviceHandle) << "No live device for UUID, loading persisted state."
                               << "uuid:" << _uuid
                               << "path:" << getResourcePath();
        loadLastData();
    }
}

DeviceHandle::DeviceHandle(QString path, DeviceManager *dm)
    : IResource(std::move(path), nullptr),
      _temporary(true),
      _lock(QReadWriteLock::Recursive),
      _deviceManager(dm)
{
    if (_deviceManager.isNull())
    {
        qCWarning(lcDeviceHandle) << "DeviceManager is null. Handle is inactive."
                                  << "path:" << getResourcePath();
        return;
    }

    connect(dm, &DeviceManager::deviceRegistered, this, &DeviceHandle::deviceRegistered);
    connect(dm, &DeviceManager::deviceDeregistered, this, &DeviceHandle::deviceDeregistered);
}

DeviceHandle::~DeviceHandle()
{
    qCDebug(lcDeviceHandle) << "Destroy handle."
                            << "path:" << getResourcePath()
                            << "uuid:" << (_uuid.isEmpty() ? QStringLiteral("<none>") : _uuid);

           // Persist only for non-temporary handles.
    {
        QReadLocker locker(&_lock);
        if (_temporary)
            return;
    }

    save();
}

// ----------------------------------------------------------------------------
// Serialization
// ----------------------------------------------------------------------------
const QVariantMap DeviceHandle::getData()
{
    QVariantMap data;
    QVariantMap properties;
    QVariantMap permissions;

    QReadLocker locker(&_lock);

    for (auto it = _properties.cbegin(); it != _properties.cend(); ++it)
        properties.insert(it.key(), it.value()->toMap());

    for (auto it = _permissions.cbegin(); it != _permissions.cend(); ++it)
        permissions.insert(it.key(), it.value());

    data["properties"] = properties;
    data["functions"] = _functions;
    data["type"] = _type;
    data["lastOnline"] = _lastOnline;
    data["description"] = _description;
    data["authkey"] = _authentificationKey;
    data["enableauthkey"] = _enableSecureCheck;
    data["shortID"] = _shortID;
    data["permissions"] = permissions;

    return data;
}

// ----------------------------------------------------------------------------
// Basic setters / getters
// ----------------------------------------------------------------------------
void DeviceHandle::setUuid(QString uuid)
{
    if (uuid == _uuid)
        return;

    {
        QWriteLocker locker(&_lock);
        _initialized = false;
        _uuid = std::move(uuid);
    }

    qCDebug(lcDeviceHandle) << "UUID changed."
                            << "uuid:" << _uuid
                            << "path:" << getResourcePath();

    Q_EMIT uuidChanged(_uuid);
}

void DeviceHandle::setDescription(QString description, QString token)
{
    Q_UNUSED(token)

    {
        QWriteLocker locker(&_lock);
        _description = std::move(description);
    }

    qCInfo(lcDeviceHandle) << "Description updated."
                           << "uuid:" << _uuid
                           << "path:" << getResourcePath();

    save();
    Q_EMIT descriptionChanged(_uuid, _description);
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

// ----------------------------------------------------------------------------
// Device binding / unbinding
// ----------------------------------------------------------------------------
bool DeviceHandle::setDevice(QSharedPointer<IDevice> device)
{
    if (!device)
        return false;

           // Replace previous device (if any).
    {
        QReadLocker locker(&_lock);
        if (_device)
        {
            locker.unlock();
            removeDevice();
        }
    }

    setUuid(device->uuid());

    connect(device.data(), &IDevice::propertyChanged, this, &DeviceHandle::propertyChangedSlot);
    connect(device.data(), &IDevice::dataReceived, this, &DeviceHandle::dataReceived);
    connect(device.data(), &IDevice::deviceStateChanged, this, &DeviceHandle::deviceStateChangedSlot);
    connect(device.data(), &IDevice::forcePropertySync, this, &DeviceHandle::syncDevice);

    QString token;
    {
        QWriteLocker locker(&_lock);
        _device = device;
        _deviceSate = device->getDeviceState();
        _firmwareVersion = device->getFirmwareVersion();
        _temporary = false;

               // Only authenticate if permissions are used.
        token = _permissions.isEmpty() ? QString() : AuthenticationService::instance()->login(device);
        _token = token;
    }

    if (!_permissions.isEmpty())
    {
        device->setGrantedPermissions(_permissions);
        device->setToken(token);
    }

    qCInfo(lcDeviceHandle) << "Device attached."
                           << "uuid:" << _uuid
                           << "state:" << _deviceSate
                           << "temporary:" << _temporary;

    Q_EMIT deviceStateChanged(_uuid, _deviceSate);
    Q_EMIT temporaryChanged(_uuid, _temporary);

    syncDevice();

    bool emitInit = false;
    {
        QWriteLocker locker(&_lock);
        if (!_initialized)
        {
            _initialized = true;
            emitInit = true;
        }
    }

    if (emitInit)
        Q_EMIT init();


    save();
    return true;
}

bool DeviceHandle::removeDevice()
{
    {
        QReadLocker locker(&_lock);
        if (!_device)
            return false;
    }

    qCInfo(lcDeviceHandle) << "Device detached." << "uuid:" << _uuid;

    setUuid("");

    disconnect(_device.data(), &IDevice::propertyChanged, this, &DeviceHandle::propertyChangedSlot);
    disconnect(_device.data(), &IDevice::dataReceived, this, &DeviceHandle::dataReceived);
    disconnect(_device.data(), &IDevice::deviceStateChanged, this, &DeviceHandle::deviceStateChangedSlot);
    disconnect(_device.data(), &IDevice::forcePropertySync, this, &DeviceHandle::syncDevice);

    {
        QWriteLocker locker(&_lock);
        _deviceSate = IDevice::OFFLINE;
        _temporary = true;
        _device = nullptr;
    }

    Q_EMIT deviceStateChanged(_uuid, _deviceSate);
    Q_EMIT temporaryChanged(_uuid, true);

    save();
    return true;
}

// ----------------------------------------------------------------------------
// Permissions / RPC / properties API
// ----------------------------------------------------------------------------
IDevice::DeviceError DeviceHandle::setDeviceProperty(QString property, QVariant value, QString token)
{
    IDevicePermissionChecker::PropertyPermission permission;
    {
        QReadLocker locker(&_lock);
        if (!_permissionChecker.isNull())
            permission = _permissionChecker->checkPropertyPermission(token, this, property);
    }

    if (!permission.canWrite)
    {
        qCWarning(lcDeviceHandle) << "Write denied."
                                  << "uuid:" << _uuid
                                  << "property:" << property;
        return IDevice::PERMISSION_DENIED;
    }

    DeviceProperty* prop = nullptr;
    {
        QReadLocker locker(&_lock);
        prop = _properties.value(property, nullptr);
    }

    if (!prop)
        return IDevice::PROPERTY_NOT_EXISTS;

    prop->setValue(std::move(value));
    return IDevice::NO_ERROR;
}

IDevice::DeviceError DeviceHandle::triggerFunction(QString name, QVariant parameters, QString token, QString cbID)
{
    bool canCall = true;
    {
        QReadLocker locker(&_lock);
        if (!token.isEmpty() && !_permissionChecker.isNull())
            canCall = _permissionChecker->checkRPCPermission(token, this, name);
    }

    if (!canCall)
    {
        qCWarning(lcDeviceHandle) << "RPC denied."
                                  << "uuid:" << _uuid
                                  << "rpc:" << name;
        return IDevice::PERMISSION_DENIED;
    }

    QReadLocker locker(&_lock);

    if (_deviceSate != IDevice::ONLINE)
        return IDevice::DEVICE_NOT_AVAILABLE;

    QVariantMap paramMap = parameters.toMap();

           // Attach caller identity if token is valid.
    const iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    if (!identity.isNull())
        paramMap["caller"] = identity.data()->identityID();

    qCDebug(lcDeviceHandle) << "Trigger RPC."
                            << "uuid:" << _uuid
                            << "rpc:" << name
                            << "cb:" << cbID;

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

    for (auto it = _properties.cbegin(); it != _properties.cend(); ++it)
        properties << it.value()->toMap();

    return properties;
}

DeviceProperty *DeviceHandle::property(QString name)
{
    QReadLocker locker(&_lock);
    return _properties.value(name, nullptr);
}

DeviceProperty* DeviceHandle::createDevicePropertyObject(QString name, DeviceHandle *parent, QVariantMap metadata)
{
    auto* prop = new DeviceProperty(std::move(name), parent, std::move(metadata));
    connect(prop, &DeviceProperty::metadataChanged, this, &DeviceHandle::save);
    return prop;
}

QVariant DeviceHandle::getPropertyValue(QString name) const
{
    QReadLocker locker(&_lock);
    const DeviceProperty* prop = _properties.value(name, nullptr);
    if (!prop)
        return QVariant(); // invalid

    return prop->getValue();
}

// ----------------------------------------------------------------------------
// Sync / persistence
// ----------------------------------------------------------------------------
void DeviceHandle::syncDevice()
{
    QVariantMap newProperties;
    {
        QReadLocker locker(&_lock);
        if (!_device)
            return;

        newProperties = _device->getProperties();
    }

    QVariantMap unconfirmedProperties;

    for (auto it = newProperties.cbegin(); it != newProperties.cend(); ++it)
    {
        const QString key = it.key();
        const QVariant val = it.value();

        DeviceProperty* property = nullptr;
        {
            QReadLocker locker(&_lock);
            property = _properties.value(key, nullptr);
        }

        if (property)
        {
            // Values come directly from the device after reattach. Keep "dirty" state unchanged.
            property->setRealValue(val, true);
        }
        else
        {
            property = createDevicePropertyObject(key, this);
            property->setRealValue(val);
            registerPropertyObject(key, property);
        }

               // Collect dirty values to re-apply on initDevice().
        {
            QReadLocker locker(&_lock);
            for (auto pit = _properties.cbegin(); pit != _properties.cend(); ++pit)
            {
                if (pit.value()->isDirty())
                    unconfirmedProperties.insert(pit.key(), pit.value()->getSetValue());
            }
        }

        Q_EMIT propertyChanged(_uuid, key, val, false);
    }

    {
        QWriteLocker locker(&_lock);

        _functions = _device->getFunctions();
        _type = _device->type();
        _permissionChecker = DevicePermissionManager::instance()->getDevicePermissionChecker(_type);
        _shortID = _device->shortId();

        qCInfo(lcDeviceHandle) << "Synced device."
                               << "uuid:" << _uuid
                               << "type:" << _type
                               << "props:" << newProperties.size()
                               << "dirty:" << unconfirmedProperties.size();

        _device->initDevice(unconfirmedProperties);
    }

    save();
}

void DeviceHandle::loadLastData()
{
    const QVariantMap data = load();
    if (data.isEmpty())
    {
        qCDebug(lcDeviceHandle) << "No persisted state found." << "path:" << getResourcePath();
        return;
    }

           // Permissions
    QMap<QString, bool> permissionMap;
    const QVariantMap map = data["permissions"].toMap();
    for (auto it = map.cbegin(); it != map.cend(); ++it)
        permissionMap.insert(it.key(), it.value().toBool());

    {
        QWriteLocker locker(&_lock);

        _type = data["type"].toString();
        _permissionChecker = DevicePermissionManager::instance()->getDevicePermissionChecker(_type);
        _functions = data["functions"].toList();
        _shortID = data["shortID"].toString();
        _description = data["description"].toString();
        _authentificationKey = data["authkey"].toUInt();
        _enableSecureCheck = data["enableauthkey"].toBool();
        _lastOnline = data["lastOnline"].toLongLong();
        _permissions = permissionMap;
    }

           // Properties
    const QVariantMap props = data["properties"].toMap();
    for (auto it = props.cbegin(); it != props.cend(); ++it)
    {
        DeviceProperty* prop = createDevicePropertyObject(it.key(), this, it.value().toMap());
        registerPropertyObject(it.key(), prop);
    }

    qCInfo(lcDeviceHandle) << "Loaded persisted state."
                           << "type:" << _type
                           << "props:" << props.size()
                           << "permissions:" << _permissions.size();
}

void DeviceHandle::registerPropertyObject(QString name, DeviceProperty *prop)
{
    {
        QWriteLocker locker(&_lock);
        _properties.insert(name, prop);
    }

    connect(prop, &DeviceProperty::setValueChanged, this, &DeviceHandle::sendPropertyToDevice);
    Q_EMIT newPropertyObject(prop);
}

void DeviceHandle::setPermissions(const QMap<QString, bool> &permissions)
{
    {
        QWriteLocker locker(&_lock);
        _permissions = permissions;
    }

    qCInfo(lcDeviceHandle) << "Permissions updated."
                           << "uuid:" << _uuid
                           << "count:" << _permissions.size();

    save();
}

QSharedPointer<IDevice> DeviceHandle::getDevice() const
{
    // Returning a shared pointer copy is thread-safe; keep state consistent under lock.
    QReadLocker locker(&_lock);
    return _device;
}

bool DeviceHandle::getEnableSecureCheck() const
{
    QReadLocker locker(&_lock);
    return _enableSecureCheck;
}

IDevice::DeviceError DeviceHandle::startFirmwareUpdate(QVariant args)
{
    QReadLocker locker(&_lock);
    if (!_device)
        return IDevice::DEVICE_NOT_AVAILABLE;

    qCInfo(lcDeviceHandle) << "Start firmware update." << "uuid:" << _uuid;
    return _device->startFirmwareUpdate(std::move(args));
}

int DeviceHandle::getFirmwareVersion()
{
    QReadLocker locker(&_lock);
    return _firmwareVersion;
}

QVariantMap DeviceHandle::getPermissions()
{
    QVariantMap map;
    QReadLocker locker(&_lock);
    for (auto it = _permissions.cbegin(); it != _permissions.cend(); ++it)
        map.insert(it.key(), it.value());
    return map;
}

quint32 DeviceHandle::getAuthentificationKey() const
{
    QReadLocker locker(&_lock);
    return _authentificationKey;
}

void DeviceHandle::setAuthentificationKey(const quint32 &securekey)
{
    {
        QWriteLocker locker(&_lock);
        _authentificationKey = securekey;
        _enableSecureCheck = true;
    }

    qCInfo(lcDeviceHandle) << "Authentication key updated." << "uuid:" << _uuid;
    save();
}

// ----------------------------------------------------------------------------
// DeviceProperty -> Device forwarding
// ----------------------------------------------------------------------------
void DeviceHandle::sendPropertyToDevice(QString name, QVariant value)
{
    const auto* senderProp = qobject_cast<DeviceProperty*>(sender());
    if (!senderProp)
        return;

    iDevicePtr device;
    {
        QReadLocker locker(&_lock);
        device = _device;
    }

    if (device)
    {
        qCDebug(lcDeviceHandle) << "Forward property to device."
                                << "uuid:" << _uuid
                                << "property:" << name;
        device->setDeviceProperty(name, value);
    }
    else
    {
        // No live device attached; persist shadow value.
        qCDebug(lcDeviceHandle) << "No device attached, persisting property shadow."
                                << "uuid:" << _uuid
                                << "property:" << name;
        save();
    }

    Q_EMIT propertyChanged(_uuid, name, value, true);
}

// ----------------------------------------------------------------------------
// Device lifecycle slots
// ----------------------------------------------------------------------------
void DeviceHandle::deviceStateChangedSlot(QString uuid, IDevice::DeviceState state)
{
    {
        QWriteLocker locker(&_lock);
        _deviceSate = state;
    }

    qCInfo(lcDeviceHandle) << "Device state changed."
                           << "uuid:" << uuid
                           << "state:" << state;

    if (state == IDevice::ONLINE)
        syncDevice();

    Q_EMIT deviceStateChanged(uuid, state);
}

void DeviceHandle::deviceDeregistered(QString uuid)
{
    {
        QReadLocker locker(&_lock);
        if (uuid != _uuid || !_device)
            return;
    }

    qCInfo(lcDeviceHandle) << "Device deregistered." << "uuid:" << uuid;

    disconnect(_device.data(), &IDevice::propertyChanged, this, &DeviceHandle::propertyChangedSlot);
    disconnect(_device.data(), &IDevice::dataReceived, this, &DeviceHandle::dataReceived);
    disconnect(_device.data(), &IDevice::deviceStateChanged, this, &DeviceHandle::deviceStateChangedSlot);

    save();

    {
        QWriteLocker locker(&_lock);
        _lastOnline = QDateTime::currentMSecsSinceEpoch();
        _deviceSate = IDevice::OFFLINE;

        AuthenticationService::instance()->logout(_token);

        _device = nullptr;
    }

    Q_EMIT deviceStateChanged(_uuid, IDevice::OFFLINE);
}

void DeviceHandle::deviceRegistered(QString uuid)
{
    {
        QReadLocker locker(&_lock);
        if (_device)
            return;
    }

    if (uuid != _uuid)
        return;

    if (_deviceManager.isNull())
    {
        qCWarning(lcDeviceHandle) << "DeviceManager is null. Abort attach."
                                  << "uuid:" << uuid
                                  << "path:" << getResourcePath();
        return;
    }

    qCInfo(lcDeviceHandle) << "Device registered, attempting attach." << "uuid:" << uuid;

    const iDevicePtr device = _deviceManager->getDeviceByUuid(uuid);
    if (!device)
    {
        qCWarning(lcDeviceHandle) << "Device registered but not retrievable by UUID."
                                  << "uuid:" << uuid;
        return;
    }

           // Optional secure check: validate authentication key and requested permissions.
    {
        QReadLocker locker(&_lock);
        if (_enableSecureCheck)
        {
            if (_authentificationKey != device->getAuthentificationKey())
            {
                qCWarning(lcDeviceHandle) << "Authentication key mismatch; device rejected."
                                          << "uuid:" << _uuid
                                          << "is:" << device->getAuthentificationKey()
                                          << "expected:" << _authentificationKey;
                return;
            }

            if (_permissions != device->getRequestedPermissions())
            {
                qCWarning(lcDeviceHandle) << "Unconfirmed permission request; device rejected."
                                          << "uuid:" << _uuid;
                return;
            }

            qCInfo(lcDeviceHandle) << "Secure check succeeded." << "uuid:" << _uuid;
        }
    }

    setDevice(device);
}

// ----------------------------------------------------------------------------
// Device -> DeviceProperty updates
// ----------------------------------------------------------------------------
void DeviceHandle::propertyChangedSlot(QString uuid, QString property, QVariant value)
{
    Q_UNUSED(uuid)

    DeviceProperty* prop = nullptr;
    {
        QReadLocker locker(&_lock);
        prop = _properties.value(property, nullptr);
    }

    if (prop)
    {
        prop->setRealValue(value);
    }
    else
    {
        prop = createDevicePropertyObject(property, this);
        prop->setRealValue(value);
        registerPropertyObject(property, prop);
    }

    Q_EMIT propertyChanged(_uuid, property, value, false);
}
