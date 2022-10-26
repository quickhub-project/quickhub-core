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
        quint32 authkey = data["key"].toUInt();
        QSharedPointer<SocketDevice> device = qSharedPointerObjectCast<SocketDevice>(DeviceManager::instance()->getDeviceByUuid(uuid));
        socket->setKeepAlive(15000, 5000); // before: 30s / 10s
        if(device)
        {
            // Even if it is a bit hacked it can not be avoided: The Auth token must be
            // checked BEFORE an authenticated device is kicked out of the session by
            // an unauthenticated device.
            deviceHandlePtr handle = DeviceManager::instance()->getHandle(uuid);
            if(handle->getAuthentificationKey() != authkey)
            {
                qWarning() << "Unauthenticated device has tried to log in!";
                return true;
            }
            // update the already existing instance with the new property values
            DeviceManager::instance()->deregisterDevice(device->uuid());
        }
        else
        {
            device = QSharedPointer<SocketDevice>(new SocketDevice());
        }

        // re-init existing device the new connection
        device->init(data, socket);
        DeviceManager::instance()->registerDevice(qSharedPointerObjectCast<IDevice>(device));

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
