/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "DeviceManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QRandomGenerator>

#include "../Authentication/AuthentificationService.h"
#include "../Authentication/User.h"

// ----------------------------------------------------------------------------
// Logging
// ----------------------------------------------------------------------------

Q_LOGGING_CATEGORY(lcDeviceManager, "quickhub.devicemanager")

// ----------------------------------------------------------------------------
// Singleton
// ----------------------------------------------------------------------------

Q_GLOBAL_STATIC(DeviceManager, deviceManager);

// ----------------------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------------------

DeviceManager::DeviceManager(QObject* parent)
    : QObject(parent)
{
}

DeviceManager::~DeviceManager() = default;

DeviceManager* DeviceManager::instance()
{
    return deviceManager;
}

// ----------------------------------------------------------------------------
// Public API
// ----------------------------------------------------------------------------

void DeviceManager::init(QString storagePath)
{
    _storagePath = std::move(storagePath);

    qCInfo(lcDeviceManager) << "Init DeviceManager, storagePath =" << _storagePath;

    loadMappings();
    loadHandles();
}

bool DeviceManager::registerDevice(iDevicePtr device)
{
    if (!device) {
        qCWarning(lcDeviceManager) << "registerDevice called with null device";
        return false;
    }

    const QString uuid = device->uuid();
    const QString shortIdUpper = device->shortId().toUpper();

    _shortIDtoUid.insert(shortIdUpper, uuid);
    _deviceMap.insert(uuid, device);

    connect(device.data(), &IDevice::deregistered, this, &DeviceManager::deregisterDevice);

    qCInfo(lcDeviceManager) << "Device registered:" << uuid << "(shortId =" << shortIdUpper << ")";

           // Handles listen to this signal. The corresponding handle will fetch it via getDeviceByUuid().
    Q_EMIT deviceRegistered(uuid);

           // If a mapping was prepared while the device was offline, try to hook now.
    if (_preparedHooks.contains(uuid)) {
        const QString mapping = _preparedHooks.value(uuid);
        const auto err = hook(mapping, uuid);
        if (err == Err::NO_ERROR) {
            _preparedHooks.remove(uuid);
            qCInfo(lcDeviceManager) << "Prepared mapping applied:" << mapping << "->" << uuid;
        } else {
            qCWarning(lcDeviceManager) << "Prepared mapping failed:" << mapping << "->" << uuid
                                       << "err =" << static_cast<int>(err);
        }
    }

    return true;
}

bool DeviceManager::exists(QString uuid)
{
    return _deviceMap.contains(uuid);
}

QStringList DeviceManager::getDevices() const
{
    return _deviceMap.keys();
}

QMap<QString, QString> DeviceManager::getMappings() const
{
    return _deviceMappings;
}

iDevicePtr DeviceManager::getDeviceByUuid(QString uuid) const
{
    return _deviceMap.value(uuid, nullptr);
}

QString DeviceManager::getDeviceByMapping(QString mapping) const
{
    return _deviceMappings.value(mapping, "");
}

QString DeviceManager::getUuidForShortId(QString shortID) const
{
    return _shortIDtoUid.value(shortID.toUpper());
}

QString DeviceManager::getTypeForUuid(QString uuid) const
{
    const iDevicePtr device = _deviceMap.value(uuid);
    return device ? device->type() : QString();
}

QList<deviceHandlePtr> DeviceManager::getHandles()
{
    return _handles.values();
}

deviceHandlePtr DeviceManager::getHandle(QString uuid) const
{
    return _handles.value(uuid, deviceHandlePtr());
}

deviceHandlePtr DeviceManager::getHandleByMapping(QString mapping)
{
    // Fast path: mapping -> uuid -> handle
    const QString uuid = _deviceMappings.value(mapping);
    if (!uuid.isEmpty()) {
        const deviceHandlePtr handle = _handles.value(uuid);
        if (!handle.isNull()) {
            return handle;
        }
    }

           // Mapping may point to a handle that is still referenced elsewhere (weak cache).
    if (_handleByMappings.contains(mapping)) {
        const weakDeviceHandlePtr weakHandle = _handleByMappings.value(mapping);
        if (!weakHandle.isNull()) {
            return weakHandle.toStrongRef();
        }
    }

           // Create a dummy handle (device not hooked yet).
    deviceHandlePtr dummy(new DeviceHandle("",this));
    _handleByMappings.insert(mapping, dummy);

    qCDebug(lcDeviceManager) << "Created dummy handle for mapping:" << mapping;
    return dummy;
}

// ----------------------------------------------------------------------------
// Authorization helpers
// ----------------------------------------------------------------------------

static bool isManageDevicesAuthorized(const QString& token)
{
    const iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    return !user.isNull() && user->isAuthorizedTo(MANAGE_DEVICES);
}

Err::CloudError DeviceManager::setDeviceMapping(QString token, QString mapping, QString uuid, bool force)
{
    if (!isManageDevicesAuthorized(token)) {
        qCWarning(lcDeviceManager) << "setDeviceMapping denied (unauthorized), mapping =" << mapping;
        return Err::PERMISSION_DENIED;
    }

    if (mapping.isEmpty()) {
        qCWarning(lcDeviceManager) << "setDeviceMapping invalid (empty mapping)";
        return Err::INVALID_DATA;
    }

           // Empty uuid means: remove existing mapping.
    if (uuid.isEmpty()) {
        return unhook(mapping);
    }

    return hook(mapping, uuid, force);
}

Err::CloudError DeviceManager::setDeviceMappingByShortId(QString token, QString mapping, QString shortID, bool force)
{
    const QString uuid = _shortIDtoUid.value(shortID.toUpper());
    if (uuid.isEmpty()) {
        qCWarning(lcDeviceManager) << "setDeviceMappingByShortId invalid shortID =" << shortID;
        return Err::INVALID_DATA;
    }

    return setDeviceMapping(token, mapping, uuid, force);
}

Err::CloudError DeviceManager::prepareDeviceMapping(QString token, QString mapping, QString uuid)
{
    if (!isManageDevicesAuthorized(token)) {
        qCWarning(lcDeviceManager) << "prepareDeviceMapping denied (unauthorized), mapping =" << mapping;
        return Err::PERMISSION_DENIED;
    }

           // If the device is offline, hook() will fail. In that case we remember the mapping.
    if (hook(mapping, uuid) != Err::NO_ERROR) {
        _preparedHooks.insert(uuid, mapping);
        qCInfo(lcDeviceManager) << "Prepared mapping stored (device offline):" << mapping << "->" << uuid;
    }

    return Err::NO_ERROR;
}

Err::CloudError DeviceManager::removeMapping(const QString& mapping)
{
    return unhook(mapping);
}

// ----------------------------------------------------------------------------
// Handle management
// ----------------------------------------------------------------------------

deviceHandlePtr DeviceManager::addDeviceHandle(QString uuid)
{
    if (_handles.contains(uuid)) {
        return _handles.value(uuid);
    }

    const QString path = _storagePath + "/handles/" + uuid;
    deviceHandlePtr handle(new DeviceHandle(uuid, path, this));
    _handles.insert(uuid, handle);

    qCInfo(lcDeviceManager) << "Device handle created:" << uuid << "path =" << path;

    Q_EMIT newDeviceHandle(uuid);
    return handle;
}

// ----------------------------------------------------------------------------
// Mapping operations
// ----------------------------------------------------------------------------

Err::CloudError DeviceManager::unhook(QString mapping)
{
    qCInfo(lcDeviceManager) << "Remove mapping:" << mapping;

    const QString deviceUUID = getDeviceByMapping(mapping);
    if (deviceUUID.isEmpty()) {
        qCWarning(lcDeviceManager) << "unhook failed: mapping not found:" << mapping;
        return Err::INVALID_DATA;
    }

    _deviceMappings.remove(mapping);
    Q_EMIT deviceMappingRemoved(deviceUUID, mapping);

           // Keep the handle alive if it is still referenced somewhere else.
    const weakDeviceHandlePtr weakPtr = _handles.value(deviceUUID).toWeakRef();
    _handles.remove(deviceUUID);

    if (!weakPtr.isNull()) {
        /* Someone still owns a smart pointer to this DeviceHandle instance.
         * The instance will be deleted when there are no more references.
         * Until then, the manager keeps a weak reference keyed by mapping.
         * This also allows reusing the same handle if the mapping is recreated. */
        weakPtr.toStrongRef()->removeDevice();
        _handleByMappings.insert(mapping, weakPtr);
        qCDebug(lcDeviceManager) << "Handle moved to weak cache for mapping:" << mapping;
    }

    Q_EMIT deviceHandleRemoved(deviceUUID);

    saveMappings();
    return Err::NO_ERROR;
}

Err::CloudError DeviceManager::hook(QString mapping, QString uuid, bool force)
{
    const iDevicePtr device = _deviceMap.value(uuid, nullptr);
    if (!device) {
        // Device needs to be online to hook.
        qCWarning(lcDeviceManager) << "hook denied: device offline, mapping =" << mapping << "uuid =" << uuid;
        return Err::PERMISSION_DENIED;
    }

           // If mapping exists, replace it.
    if (_deviceMappings.contains(mapping)) {
        qCInfo(lcDeviceManager) << "hook overwrites existing mapping:" << mapping;
        unhook(mapping);
    }

           // Ensure a device is only mapped once.
    if (_deviceMappings.values().contains(uuid)) {
        if (force) {
            const QString oldMapping = _deviceMappings.key(uuid);
            qCInfo(lcDeviceManager) << "hook force-removes existing device mapping:" << oldMapping << "->" << uuid;
            unhook(oldMapping);
        } else {
            qCWarning(lcDeviceManager) << "hook failed: device already mapped, uuid =" << uuid;
            return Err::ALREADY_EXISTS;
        }
    }

           // Optional authentication key.
    quint32 authKey = 0;
    if (device->enableAuthentificationKey()) {
        authKey = QRandomGenerator::global()->generate();
        device->setAuthentificationKey(authKey);
        qCDebug(lcDeviceManager) << "Auth key generated for device uuid =" << uuid;
    }

    _deviceMappings.insert(mapping, uuid);

           // If a dummy handle exists, bind it to the real device now.
    deviceHandlePtr handle = _handleByMappings.value(mapping);
    if (!handle.isNull()) {
        const QString path = _storagePath + "/handles/" + uuid;
        handle->setResourcePath(path);
        handle->setDevice(_deviceMap[uuid]);

        if (authKey != 0) {
            handle->setAuthentificationKey(authKey);
        }

        _handles.insert(uuid, handle);
        Q_EMIT newDeviceHandle(uuid);

        qCInfo(lcDeviceManager) << "Dummy handle bound to device:" << uuid << "mapping =" << mapping;
    } else {
        qCDebug(lcDeviceManager) << "Create new handle for uuid =" << uuid;
        handle = addDeviceHandle(uuid);

        if (!handle.isNull() && authKey != 0) {
            handle->setAuthentificationKey(authKey);
        }
    }

    handle->setPermissions(device->getRequestedPermissions());

    Q_EMIT newDeviceMapping(uuid, mapping);

    _handleByMappings.remove(mapping);

    saveMappings();

    qCInfo(lcDeviceManager) << "Mapping created:" << mapping << "->" << uuid
                            << "(force =" << force << ")";

    return Err::NO_ERROR;
}

// ----------------------------------------------------------------------------
// Persistence
// ----------------------------------------------------------------------------

void DeviceManager::saveMappings()
{
    const QString filePath = _storagePath + "/mappings";
    qCInfo(lcDeviceManager) << "Save mappings to:" << filePath;

    QVariantMap mappings;
    for (auto it = _deviceMappings.constBegin(); it != _deviceMappings.constEnd(); ++it) {
        mappings.insert(it.key(), it.value());
    }

    QVariantMap data;
    data["mappings"] = mappings;

    QFile file(filePath);
    QFileInfo info(file);

    QDir dir(info.absolutePath());
    if (!dir.exists() && !dir.mkpath(info.absolutePath())) {
        qCWarning(lcDeviceManager) << "Could not create directory:" << info.absolutePath();
        return;
    }

    if (!file.open(QFile::WriteOnly)) {
        qCWarning(lcDeviceManager) << "Could not open mappings file for writing:" << filePath
                                   << "error =" << file.errorString();
        return;
    }

    file.write(QJsonDocument::fromVariant(data).toJson());
    file.close();
}

void DeviceManager::loadMappings()
{
    const QString filePath = _storagePath + "/mappings";
    QFile file(filePath);

    if (!file.open(QFile::ReadOnly)) {
        qCWarning(lcDeviceManager) << "Could not open mappings file for reading:" << filePath
                                   << "error =" << file.errorString();
        return;
    }

    const QVariantMap data = QJsonDocument::fromJson(file.readAll()).toVariant().toMap();
    const QVariantMap mappings = data.value("mappings").toMap();
    file.close();

    _deviceMappings.clear();

    for (auto it = mappings.constBegin(); it != mappings.constEnd(); ++it) {
        const QString mapping = it.key();
        const QString uuid = it.value().toString();

        _deviceMappings.insert(mapping, uuid);
        Q_EMIT newDeviceMapping(uuid, mapping);
    }

    qCInfo(lcDeviceManager) << "Loaded mappings:" << _deviceMappings.size();
}

void DeviceManager::loadHandles()
{
    // Load only handles for devices that have a mapping.
    const QList<QString> values = _deviceMappings.values();
    const QSet<QString> registeredUuids(values.begin(), values.end());

    qCInfo(lcDeviceManager) << "Load handles for mapped devices:" << registeredUuids.size();

    for (const QString& uuid : registeredUuids) {
        qCDebug(lcDeviceManager) << "Load handle for uuid =" << uuid;
        addDeviceHandle(uuid);
    }
}

// ----------------------------------------------------------------------------
// Device lifecycle callbacks
// ----------------------------------------------------------------------------

void DeviceManager::deregisterDevice(QString uuid)
{
    if (!_deviceMap.contains(uuid)) {
        qCDebug(lcDeviceManager) << "deregisterDevice ignored (unknown uuid):" << uuid;
        return;
    }

    disconnect(_deviceMap.value(uuid).data(), &IDevice::deregistered, this, &DeviceManager::deregisterDevice);
    _deviceMap.remove(uuid);

    qCInfo(lcDeviceManager) << "Device deregistered:" << uuid;

    Q_EMIT deviceDeregistered(uuid);
}
