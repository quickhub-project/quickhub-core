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

QVariantMap DeviceListWrapper::toMap(IDevice *device) const
{
    bool isRegistered = !DeviceManager::instance()->getMappings().keys(device->uuid()).isEmpty();
    QVariantMap deviceData;
    deviceData["online"] = device->getDeviceState() == IDevice::ONLINE;
    deviceData["isRegistered"] = isRegistered;
    deviceData["type"] = device->type();
    deviceData["uuid"] = device->uuid();
    deviceData["shortID"] = device->shortId();

    return deviceData;
}

bool DeviceListWrapper::addDevice(QString uuid)
{
    IDevice* device = DeviceManager::instance()->getDeviceByUuid(uuid);
    if(device)
    {
        connect(device, &IDevice::deviceStateChanged, this, &DeviceListWrapper::deviceStateChanged);
        _list.insert(uuid, toMap(device));
        return true;
    }
    return false;
}

int DeviceListWrapper::getIndex(QString uuid) const
{
    if(!_list.contains(uuid))
        return -1;

    QList<QString> keys = _list.keys();
    auto i = qBinaryFind(keys.begin(), keys.end(), uuid);
    return i - keys.begin();
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

void DeviceListWrapper::devicePropertyChanged(QString uuid, QString property, QVariant value)
{
    if(!_list.contains(uuid))
        return;

    int idx = getIndex(uuid);
    QVariantMap item = _list[uuid].toMap();
    QVariantMap properties = item["properties"].toMap();
    properties[property] = value;
    item["properties"] = properties;
    _list.insert(uuid, item);
    Q_EMIT propertyChanged("properties", properties,idx);
}


void DeviceListWrapper::newDevice(QString uuid)
{
    if(addDevice(uuid))
    {
        int idx = getIndex(uuid);
        Q_EMIT itemAdded(_list.value(uuid), idx);
    }
}

void DeviceListWrapper::deviceRemoved(QString uuid)
{
    int index = getIndex(uuid);
    if(index < 0)
        return;

    _list.remove(uuid);
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
    QVariantMap item = _list[deviceUUID].toMap();
    item["isRegistered"] = isRegistered;
    _list.insert(deviceUUID, item);
    Q_EMIT propertyChanged("isRegistered", isRegistered, getIndex(uuid));
}

void DeviceListWrapper::mappingRemoved(QString uuid, QString mapping)
{
    newMapping(uuid, mapping);
}

void DeviceListWrapper::deviceStateChanged(QString uuid, IDevice::DeviceState state)
{
    QVariantMap item = _list[uuid].toMap();
    bool online = IDevice::ONLINE == state;
    item["online"] = online;
    _list.insert(uuid, item);
    Q_EMIT propertyChanged("online", online, getIndex(uuid));
}

QVariantList DeviceListWrapper::getListData() const
{
    return _list.values();
}
