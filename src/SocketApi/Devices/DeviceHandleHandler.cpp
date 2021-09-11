/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "DeviceHandleHandler.h"
#include "Server/Devices/DeviceProperty.h"

DeviceHandleHandler::DeviceHandleHandler(QSharedPointer<DeviceHandle> deviceHandle, QObject* parent):IResourceHandler("device", parent),
    _deviceHandle(deviceHandle)
{
    connect(deviceHandle.data(), &DeviceHandle::propertyChanged, this, &DeviceHandleHandler::propertyChanged);
    connect(deviceHandle.data(), &DeviceHandle::temporaryChanged, this, &DeviceHandleHandler::temporaryChanged);
    connect(deviceHandle.data(), &DeviceHandle::deviceStateChanged, this, &DeviceHandleHandler::deviceStateChangedSlot);
    connect(deviceHandle.data(), &DeviceHandle::dataReceived, this, &DeviceHandleHandler::dataReceived);
    connect(deviceHandle.data(), &DeviceHandle::init, this, &DeviceHandleHandler::reinit);
    connect(deviceHandle.data(), &DeviceHandle::newPropertyObject, this, &DeviceHandleHandler::registerProperty);
    connect(deviceHandle.data(), &DeviceHandle::descriptionChanged, this, &DeviceHandleHandler::deviceDescriptionChanged);

    QMapIterator<QString, DeviceProperty*> it(_deviceHandle->propertyObjects());
    while(it.hasNext())
    {
        registerProperty(it.next().value());
    }
}

void DeviceHandleHandler::initHandle(ISocket *handle)
{
    handle->sendVariant(getDumpMessage());
}


void DeviceHandleHandler::handleMessage(QVariant message, ISocket *handle)
{
    QVariantMap msg         = message.toMap();
    QString     command     = msg["command"].toString();
    QString     token       = msg["token"].toString();
    QVariantMap parameters  = msg["parameters"].toMap();

    if(command == "device:call")
    {
        QString functionName = parameters["funcname"].toString();
        QVariantMap functionParameters = parameters["funcparams"].toMap();
        QString cbID  = parameters["cbID"].toString();
        int returnVal = _deviceHandle->triggerFunction(functionName, functionParameters, token, cbID);
        QVariantMap msg;
        msg["errorcode"] = returnVal > 0 ? 0 : returnVal;
        if(returnVal == IDevice::NO_ERROR)
        {
            if(!cbID.isEmpty())
            {
                _cbMap.insert(cbID, handle);
                connect(handle, &ISocket::disconnected, this, &DeviceHandleHandler::socketDisconnectedSlot, Qt::UniqueConnection);
            }
            msg["command"] = "device:call:success";
            handle->sendVariant(msg);
            return;
        }
        else
            msg["command"] = "device:call:failed";

        if(returnVal == IDevice::FUNCTION_NOT_EXIST)
            msg["errorstring"] = "Unknown function.";

        if(returnVal == IDevice::DEVICE_NOT_AVAILABLE)
            msg["errorstring"] = "Device is offline.";

        if(returnVal == IDevice::PERMISSION_DENIED)
            msg["errorstring"] = "Permission Denied.";

        handle->sendVariant(msg);
        return;
    }

    if(command == "device:setproperty")
    {
        QString property = parameters["property"].toString();
        QVariant value = parameters["value"];
        _deviceHandle->setDeviceProperty(property, value, token);
        return;
    }

    if(command == "device:description")
    {
        QString description = parameters["desc"].toString();
        _deviceHandle->setDescription(description, token);
        return;
    }


    if(command == "device:meta:set")
    {
        QString propName = parameters.firstKey();
        DeviceProperty* prop = _deviceHandle->property(propName);
        if(prop != nullptr)
        {
            QVariantMap values = parameters.first().toMap();
            QMapIterator<QString, QVariant> it(values);
            while (it.hasNext())
            {
                it.next();
                prop->setMetadata(it.key(), it.value());
            }
        }
        else
        {
            msg["errorstring"] = "Property does not exist.";
            msg["errorcode"] = IDevice::PROPERTY_NOT_EXISTS;
            handle->sendVariant(msg);
        }
    }
}

QVariantMap DeviceHandleHandler::getDumpMessage()
{
    QVariantMap msg;
    msg["command"] = "device:dump";

    QVariantMap parameters;

    parameters["props"] = _deviceHandle->properties();
    parameters["funcs"] = _deviceHandle->getFunctions();
    parameters["permissions"] = _deviceHandle->getPermissions();
    parameters["type"] = _deviceHandle->type();
    parameters["desc"] = _deviceHandle->getDescription();
    parameters["uuid"] = _deviceHandle->uuid();
    parameters["suid"] = _deviceHandle->shortUid();
    parameters["tmp"] = _deviceHandle->temporary();
    parameters["on"] = _deviceHandle->getDeviceState() == IDevice::ONLINE;
    msg["parameters"] = parameters;
    return msg;
}

void DeviceHandleHandler::registerProperty(DeviceProperty *property)
{
    connect(property, &DeviceProperty::confirmed, this, &DeviceHandleHandler::confirmedChanged);
    connect(property, &DeviceProperty::setValueChanged, this, &DeviceHandleHandler::setValueChanged);
    //connect(property, &DeviceProperty::dirtyChanged, this, &DeviceHandleHandler::dirtyChanged);
    connect(property, &DeviceProperty::realValueChanged, this, &DeviceHandleHandler::realValueChanged);
    connect(property, &DeviceProperty::metadataChanged, this, &DeviceHandleHandler::metadataChanged);
}

void DeviceHandleHandler::dataReceived(QString uuid, QString subject, QVariantMap data)
{
    Q_UNUSED(uuid)
    QVariantMap msg;
    msg["command"] = "device:data";
    QVariantMap parameters;
    parameters["subj"] = subject;
    if(!data.isEmpty())
        parameters["data"] = data;

    msg["parameters"] = parameters;
    if(_cbMap.contains(subject))
    {
        ISocket* socket = _cbMap.take(subject);
        socket->sendVariant(msg);
        return;
    }
    deployToAll(msg);
}

void DeviceHandleHandler::propertyChanged(QString uuid, QString property, QVariant value, bool dirty)
{
    Q_UNUSED(uuid)
    QVariantMap msg;
    msg["command"] = "device:property:set";
    QVariantMap parameters;
    parameters["property"] = property;
    parameters["value"] = value;
    parameters["dirty"] = dirty;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void DeviceHandleHandler::deviceStateChangedSlot(QString uuid, IDevice::DeviceState state)
{
    Q_UNUSED(uuid)
    QVariantMap msg;
    msg["command"] = "device:statuschanged";
    QVariantMap parameters;
    parameters["online"] = state == IDevice::ONLINE;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void DeviceHandleHandler::deviceDescriptionChanged(QString uuid, QString description)
{
    Q_UNUSED(uuid)
    QVariantMap msg;
    msg["command"] = "device:description";
    QVariantMap parameters;
    parameters["desc"] = description;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void DeviceHandleHandler::temporaryChanged(QString uuid, bool temporary)
{
    Q_UNUSED(uuid)
    QVariantMap msg;
    msg["command"] = "device:tmpchanged";
    QVariantMap parameters;
    parameters["tmp"] = temporary;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void DeviceHandleHandler::metadataChanged(QString name, QString key, QVariant value)
{
    QVariantMap msg;
    msg["command"] = "device:meta:set";
    QVariantMap parameters;
    QVariantMap data;
    data[key] = value;
    parameters[name] = data;
    msg["parameters"] = parameters;

    deployToAll(msg);
}


void DeviceHandleHandler::confirmedChanged(QString name, qlonglong timestamp, bool accepted)
{
    QVariantMap msg;
    msg["command"] = "device:prop:conf";
    QVariantMap data;
    data["confTS"] = timestamp;
    data["accepted"] = accepted;
    QVariantMap parameters;
    parameters[name] = data;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void DeviceHandleHandler::setValueChanged(QString name, QVariant setValue,  bool dirty)
{
    QVariantMap msg;
    msg["command"] = "device:prop:set";
    QVariantMap data;
    data["set"] = setValue;
    data["dirty"] = dirty;
    QVariantMap parameters;
    parameters[name] = data;
    msg["parameters"] = parameters;
    deployToAll(msg);
}


void DeviceHandleHandler::realValueChanged(QString name, QVariant realValue, bool dirty)
{
    QVariantMap msg;
    msg["command"] = "device:prop:set";
    QVariantMap data;
    data["real"] = realValue;
    data["dirty"] = dirty;
    QVariantMap parameters;
    parameters[name] = data;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void DeviceHandleHandler::socketDisconnectedSlot()
{
    ISocket* handle = qobject_cast<ISocket*>(sender());
    if(handle)
    {
        QStringList keys = _cbMap.keys(handle);
        if(keys.isEmpty())
        {
            QListIterator<QString> keyIt(keys);
            while(keyIt.hasNext())
                _cbMap.remove(keyIt.next());
        }
    }
}


void DeviceHandleHandler::reinit()
{
    deployToAll(getDumpMessage());
}
