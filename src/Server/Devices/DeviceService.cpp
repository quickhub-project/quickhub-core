/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "DeviceService.h"
#include "Server/Authentication/AuthentificationService.h"
#include "DeviceManager.h"
#include <QDebug>

DeviceService::DeviceService(QObject* parent) : IService(parent),
    _updateLogic(new DeviceUpdateLogic(this))
{
    connect(_updateLogic, &DeviceUpdateLogic::sendResult, this, &DeviceService::response);
}

QString DeviceService::getServiceName() const
{
    return "devices";
}

QStringList DeviceService::getServiceCalls() const
{
    QStringList calls;
    calls << "hookWithShortID" << "unhookWithShortID" << "checkForUpdates" << "startUpdate";
    return calls;
}

bool DeviceService::call(QString call, QString token, QString cbID, QVariant argument)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if (call == "checkForUpdates")
    {
        QString mapping = argument.toMap()["mapping"].toString();
        _updateLogic->checkForUpdates(mapping, cbID);
        return true;
    }

    if (call == "startUpdate")
    {
        QString mapping = argument.toMap()["mapping"].toString();
        QString url = argument.toMap()["url"].toString();
        _updateLogic->startUpdate(token, mapping, url, cbID);
        return true;
    }

    QVariantMap answer = syncCalls(call, token, argument);
    Q_EMIT response(cbID, answer);
    return true;
}

QVariantMap DeviceService::syncCalls(QString call, QString token, QVariant argument)
{
    QVariantMap argMap = argument.toMap();
    QVariantMap answer;

    if(call == "hookWithShortID")
    {
        QString shortID = argMap["shortID"].toString();
        QString mapping = argMap["mapping"].toString();
        bool force = argMap["force"].toBool();

        if(mapping.isEmpty() || shortID.isEmpty())
        {
            answer["errorstring"] = "Invalid arguments";
            answer["errorcode"] = Err::INVALID_DATA;
            return answer;
        }


        answer["errorcode"] = DeviceManager::instance()->setDeviceMappingByShortId(token, mapping, shortID, force);
        return answer;
    }

    if(call == "unhookWithShortID")
    {
        QString shortID = argMap["shortID"].toString();

        if(shortID.isEmpty())
        {
            answer["errorstring"] = "Invalid arguments";
            answer["errorcode"] = Err::INVALID_DATA;
            return answer;
        }

        QString uuid = DeviceManager::instance()->getUuidForShortId(shortID);
        QString existingMapping = DeviceManager::instance()->getMappings().key(uuid);

        answer["errorcode"] = DeviceManager::instance()->setDeviceMapping(token, existingMapping, "", true);
        return answer;
    }


    if(call == "unhookWithMapping")
    {
        QString mapping = argMap["mapping"].toString();

        if(mapping.isEmpty())
        {
            answer["errorstring"] = "Invalid arguments";
            answer["errorcode"] = Err::INVALID_DATA;
            return answer;
        }

        answer["errorcode"] = DeviceManager::instance()->setDeviceMapping(token, mapping, "", true);
        return answer;
    }


    if(call == "prepareMappingWithUUID")
    {
        QString mapping = argMap["mapping"].toString();
        QString uuid = argMap["uuid"].toString();

        if(mapping.isEmpty())
        {
            answer["errorstring"] = "Invalid arguments";
            answer["errorcode"] = Err::INVALID_DATA;
            return answer;
        }

        answer["errorcode"] = DeviceManager::instance()->prepareDeviceMapping(token, mapping, uuid);
        return answer;
    }

    return QVariantMap();
}
