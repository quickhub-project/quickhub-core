/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "SocketDevice.h"
#include <QDebug>
#include <QProcessEnvironment>

SocketDevice::SocketDevice(QObject *parent) : IDevice(parent)
{
    _ackTimer.setSingleShot(true);
    _ackTimer.setInterval(300);
    connect(&_ackTimer, &QTimer::timeout, this, &SocketDevice::sendAck);
}

void SocketDevice::init(QVariantMap data, ISocket *handle)
{
    _macID = data["id"].toString();
    _shortID = data["sid"].toString();
    _type = data["type"].toString();
    _authKey = data["key"].toUInt();

    _uuid = _macID;
    _functions = data["functions"].toList();

    QListIterator<QVariant> funcIt(_functions);
    while(funcIt.hasNext())
    {
       QVariantMap func =  funcIt.next().toMap();
       QString name = func["name"].toString();
       QVariantMap parameters = func["params"].toMap();
       _functionParameters[name] = parameters;
    }

    _properties = data["properties"].toMap();
    if(_deviceConnection != nullptr)
    {
        qWarning()<<"Already connected device has reregistered.";
        disconnect(_deviceConnection, &ISocket::messageReceived, this, &SocketDevice::messageReceived);
        disconnect(_deviceConnection, &ISocket::disconnected, this, &SocketDevice::connectionDisconnected);
        delete _deviceConnection;
        _deviceConnection = nullptr;
    }

    _deviceConnection = handle;
    handle->setParent(this);
    connect(handle, &ISocket::messageReceived, this, &SocketDevice::messageReceived);
    connect(handle, &ISocket::disconnected, this, &SocketDevice::connectionDisconnected);

    //these uuidChanged and typeChanged properties are still unused - maybe remove it?

    Q_EMIT uuidChanged();
    Q_EMIT typeChanged();
    Q_EMIT forcePropertySync();
}

QString SocketDevice::uuid() const
{
    return _uuid;
}

QString SocketDevice::shortId() const
{
    return _shortID;
}

QString SocketDevice::type() const
{
    return _type;
}

QVariantMap SocketDevice::getProperties() const
{
    return _properties;
}

QVariantList SocketDevice::getFunctions() const
{
    return _functions;
}

QString SocketDevice::macID() const
{
    return _macID;
}

void SocketDevice::messageReceived(QVariant msg)
{
    QVariantMap message = msg.toMap();
    QString command = message["cmd"].toString();
    QVariantMap parameters = message["params"].toMap();

  //  QString ackDeviceMessages = QProcessEnvironment::systemEnvironment().value("DEVICE_ACK","false");
    _ackTimer.start();

    if(command == "msg")
    {
        QString subject = parameters["subject"].toString();
        qInfo()<<"Message Received: "<< subject;
        QVariantMap data = parameters.value("data", QVariant()).toMap();
        Q_EMIT dataReceived(_uuid, subject, data);
        return;
    }

    if(command == "set")
    {
        QMapIterator<QString, QVariant> it(parameters);
        while(it.hasNext())
        {
            it.next();
            QString property = it.key();
            QVariant data = it.value();
            _properties[property] = data;
            Q_EMIT propertyChanged(_uuid, property, data);
        }

        return;
    }
}

void SocketDevice::connectionDisconnected()
{
    //handle will be deleted by SocketServer

	if (_ackTimer.isActive() )
	{
		_ackTimer.stop();
	}

    _deviceConnection = nullptr;
    Q_EMIT deregistered(_uuid);
    this->deleteLater();
}

void SocketDevice::sendAck()
{
	if ( _deviceConnection != nullptr )
	{
		QVariantMap  ack;
		ack["command"] = "ACK";
		_deviceConnection->getConnection()->sendVariant(ack);
	}
}

IDevice::DeviceError SocketDevice::triggerFunction(QString name, QVariant parameters, QString cbID)
{
    if(_ackTimer.isActive())
    {
        _ackTimer.stop();
    }

    if(!_deviceConnection)
    {
        return DEVICE_NOT_AVAILABLE;
    }

    if(!_functionParameters.contains(name))
    {
        return FUNCTION_NOT_EXIST;
    }

    QVariantMap functionInfo = _functionParameters.value(name);
    QVariantMap msg;
    msg["cmd"] = "call";
    QVariantMap params;
    params[name] = parameters;
    if(!cbID.isEmpty())
        msg["cbID"] = cbID;

    msg["params"] = params;
    _deviceConnection->sendVariant(msg);
    return NO_ERROR;
}

IDevice::DeviceError SocketDevice::setDeviceProperty(QString property, QVariant value)
{
    QString call = getPropertySetterFunc(property);
    QVariantMap params;
    params["val"] = value;
    return triggerFunction(call, params);
}

IDevice::DeviceError SocketDevice::initDevice(QVariantMap properties)
{
    if(!_deviceConnection)
        return DEVICE_NOT_AVAILABLE;

    QVariantMap msg;
    msg["cmd"] = "init";

    QVariantList functions;
    QMapIterator<QString, QVariant> it(properties);
    while(it.hasNext())
    {
        it.next();
        QVariantMap function;
        QString funcName = getPropertySetterFunc(it.key());

        if(!_functionParameters.contains(funcName))
            break;

        function["func"] =  funcName;
        QVariantMap value;
        value["val"] = it.value();
        function["args"] = value;
        functions << function;
    }

    msg["params"] = functions;
    _deviceConnection->sendVariant(msg);
    return NO_ERROR;
}

IDevice::DeviceState SocketDevice::getDeviceState()
{
    return DeviceState::ONLINE;
}

void SocketDevice::setAuthentificationKey(quint32 key)
{
    QVariantMap msg;
    msg["cmd"] = "setkey";
    msg["params"] = key;
    _deviceConnection->sendVariant(msg);
}

quint32 SocketDevice::getAuthentificationKey()
{
    return _authKey;
}

bool SocketDevice::enableAuthentificationKey()
{
    return true;
}

IDevice::DeviceError SocketDevice::startFirmwareUpdate(QVariant args)
{
    return triggerFunction(".fwupdate", args);
}

int SocketDevice::getFirmwareVersion() const
{
    if(!_properties.contains(".fwvers"))
        return -1;

    QString version = _properties[".fwvers"].toString();

    if(version.isEmpty())
        return -2;

    int major = 0;
    int minor = 0;


    QStringList split = version.split(".");
    if(split.count() > 1)
    {
        major = split[0].toInt();
        minor = split[1].toInt();
    }

    return major * 1000 + minor;
}

QString SocketDevice::getPropertySetterFunc(QString propertyName) const
{
    QChar firstChar = propertyName.at(0).toUpper();
    return "set" + propertyName.replace(0,1,firstChar);
}

