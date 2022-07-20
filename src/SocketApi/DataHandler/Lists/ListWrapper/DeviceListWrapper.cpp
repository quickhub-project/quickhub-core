/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "DeviceListWrapper.h"
#include <QVariantMap>
#include "Server/Devices/DeviceManager.h"



DeviceListWrapper::DeviceListWrapper(QObject* parent) : IList(parent)
{
    DeviceManager* manager = DeviceManager::instance();

    connect(manager, &DeviceManager::deviceRegistered, this, &DeviceListWrapper::newDevice);
    connect(manager, &DeviceManager::deviceDeregistered, this, &DeviceListWrapper::deviceRemoved);
    connect(manager, &DeviceManager::newDeviceMapping, this, &DeviceListWrapper::newMapping);
    connect(manager, &DeviceManager::deviceMappingRemoved, this, &DeviceListWrapper::mappingRemoved);

    QStringList devices = manager->getDevices();
    QListIterator<QString> deviceIt(devices);
    while(deviceIt.hasNext())
    {
        QString deviceID = deviceIt.next();
        addDevice(deviceID);
    }
}

QVariantMap DeviceListWrapper::toMap(QSharedPointer<IDevice> device) const
{
    bool isRegistered = !DeviceManager::instance()->getMappings().keys(device->uuid()).isEmpty();
    QVariantMap deviceData;
    deviceData["online"] = device->getDeviceState() == IDevice::ONLINE;
    deviceData["isRegistered"] = isRegistered;
    deviceData["type"] = device->type();
    deviceData["uuid"] = device->uuid();
    deviceData["shortID"] = device->shortId();
    QVariantMap permVariantMap;
    auto permMap = device->getRequestedPermissions();
    QMapIterator<QString, bool> it(permMap);
    while(it.hasNext())
    {
        it.next();
        permVariantMap.insert(it.key(), it.value());
    }
    deviceData["permissions"] = permVariantMap;
    return deviceData;
}

bool DeviceListWrapper::addDevice(QString uuid)
{
    iDevicePtr device = DeviceManager::instance()->getDeviceByUuid(uuid);
    if(device)
    {
        connect(device.data(), &IDevice::deviceStateChanged, this, &DeviceListWrapper::deviceStateChanged);
        _list.append(toMap(device));
        return true;
    }
    return false;
}

int DeviceListWrapper::getIndex(QString uuid) const
{

    QListIterator<QVariant>it(_list);
    int idx = 0;
    while(it.hasNext())
    {
        if(it.next().toMap()["uuid"] == uuid)
            return idx;
        idx++;
    }
    return -1;
}

void DeviceListWrapper::handleMessage(QVariant msg, ISocket *handle)
{
    QVariantMap message = msg.toMap();
    QString command = message["command"].toString();
    QString token = message["token"].toString();

    if(command == "mapping:set")
    {
        QVariantMap parameters = message["parameters"].toMap();
        QString uuid = parameters["uuid"].toString();
        QString mapping = parameters["mapping"].toString();
        Err::CloudError error = DeviceManager::instance()->setDeviceMapping(token, mapping, uuid);

        QVariantMap answer;
        if(error == Err::PERMISSION_DENIED)
        {
            answer["command"] = "mapping:set:failed";
            answer["errorcode"] = error;
            answer["errorstring"] = "Permission denied.";
            handle->sendVariant(answer);
        }

        if(error == Err::NO_ERROR)
        {
            answer["command"] = "mapping:set:success";
            handle->sendVariant(answer);
        }
    }
}

void DeviceListWrapper::newDevice(QString uuid)
{
    if(addDevice(uuid))
    {
        int idx = getIndex(uuid);
        if(idx < 0)
            return;

        qDebug()<<uuid<<"  "<<idx;
        Q_EMIT itemAdded(_list[idx], idx);
    }
}

void DeviceListWrapper::deviceRemoved(QString uuid)
{
    int index = getIndex(uuid);
    if(index < 0)
        return;

    _list.removeAt(index);
    Q_EMIT itemRemoved(index);
}

void DeviceListWrapper::newMapping(QString uuid, QString mapping)
{
    QString deviceUUID;
    if(uuid.isEmpty())
    {   
        deviceUUID = DeviceManager::instance()->getDeviceByMapping(mapping);
        if(deviceUUID.isEmpty())
            return;
    }
    else
    {
        deviceUUID = uuid;
    }


    bool isRegistered = !DeviceManager::instance()->getMappings().keys(deviceUUID).isEmpty();
    int idx = getIndex(deviceUUID);

    if(idx < 0)
        return;

    QVariantMap item = _list[idx].toMap();
    item["isRegistered"] = isRegistered;
    _list.replace(idx, item);
    Q_EMIT propertyChanged("isRegistered", isRegistered, idx);
}

void DeviceListWrapper::mappingRemoved(QString uuid, QString mapping)
{
    newMapping(uuid, mapping);
}

void DeviceListWrapper::deviceStateChanged(QString uuid, IDevice::DeviceState state)
{
    int idx = getIndex(uuid);
    if(idx < 0)
        return;

    QVariantMap item = _list[idx].toMap();
    bool online = IDevice::ONLINE == state;
    item["online"] = online;
    _list.replace(idx, item);
    Q_EMIT propertyChanged("online", online, idx);
}

QVariantList DeviceListWrapper::getListData() const
{
    return _list;
}
