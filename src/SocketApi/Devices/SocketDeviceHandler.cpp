/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "SocketDeviceHandler.h"
#include "SocketDevice.h"
#include "Server/Devices/DeviceManager.h"

SocketDeviceHandler::SocketDeviceHandler(QObject *parent) : IRequestHandler(parent)
{

}

bool SocketDeviceHandler::handleRequest(QVariantMap message, ISocket *socket)
{
    QString command = message["command"].toString();
    if(command == "node:register")
    {
        QVariantMap data = message["parameters"].toMap();
        QString uuid = data["id"].toString();
        // TODO check key
        SocketDevice* device = qobject_cast<SocketDevice*>(DeviceManager::instance()->getDeviceByUuid(uuid));
        socket->setKeepAlive(15000, 5000); // before: 30s / 10s
        if(device != nullptr)
        {
            //update the already existing handle with the new property values
            device->init(message["parameters"].toMap(), socket);
        }
        else
        {
            device = new SocketDevice();
            device->init(message["parameters"].toMap(), socket);
            DeviceManager::instance()->registerDevice(device);
        }

        return true;
    }
    return false;
}

QStringList SocketDeviceHandler::getSupportedCommands()
{
    QStringList commands;
    commands << "node:register";
    return commands;
}
