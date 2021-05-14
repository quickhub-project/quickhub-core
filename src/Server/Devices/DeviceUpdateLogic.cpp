/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "DeviceUpdateLogic.h"
#include "QProcessEnvironment"
#include "DeviceManager.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/IUser.h"


DeviceUpdateLogic::DeviceUpdateLogic(QObject *parent) : QObject(parent),
    _nam(new QNetworkAccessManager(this))
{
    _firmwareLookup = QProcessEnvironment::systemEnvironment().value("FIRMWARE_UPDATE_LOOKUP", "");
}

bool DeviceUpdateLogic::checkForUpdates(QString mapping, QString cbID)
{
    deviceHandlePtr handle = DeviceManager::instance()->getHandleByMapping(mapping);

    if(handle.isNull())
    {
        QVariantMap answer;
        answer["status"] = CHECK_FAILED;
        answer["error"] = "Handle not found.";
        Q_EMIT sendResult(cbID, answer);
        return false;
    }

    QString deviceType = handle->type();
    QUrl url(_firmwareLookup+handle->type()+"/version.json");

    QNetworkReply* reply =  _nam->get(QNetworkRequest(url));
    reply->setProperty("cbID", cbID);
    reply->setProperty("mapping", mapping);
    connect(reply, &QNetworkReply::finished, this, &DeviceUpdateLogic::lookupFinished);
    return true;
}

bool DeviceUpdateLogic::startUpdate(QString token, QString mapping, QString url,  QString cbID)
{
    QVariantMap answer;
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if (user.isNull() || !user->isAuthorizedTo(IS_ADMIN))
    {
        answer["status"] = PERMISSION_DENIED;
        answer["error"] = "Permission denied!";
        Q_EMIT sendResult(cbID, answer);
        return false;
    }

    deviceHandlePtr handle = DeviceManager::instance()->getHandleByMapping(mapping);
    if(handle.isNull() || handle->getDeviceState() != IDevice::ONLINE)
    {
       answer["status"] = DEVICE_NOT_AVAILABLE;
       Q_EMIT sendResult(cbID, answer);
       return false;
    }

    QVariantMap map;
    map["val"] = url;
    handle->startFirmwareUpdate(map);
    answer["status"] = CMD_UPDATE_SENT;
    Q_EMIT sendResult(cbID, answer);
    return true;
}

void DeviceUpdateLogic::lookupFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply)
        return;

    QString cbID = reply->property("cbID").toString();
    QString mapping = reply->property("mapping").toString();

    QVariantMap answer;
    answer["status"] = CHECK_FAILED;


    if(reply->error() != QNetworkReply::NoError)
    {
        answer["error"] = reply->errorString();
        delete reply;
        reply = nullptr;
        Q_EMIT sendResult(cbID, answer);
        return;
    }

    QVariantMap versionInfo = QJsonDocument::fromJson(reply->readAll()).toVariant().toMap();
    delete reply;
    reply = nullptr;

    deviceHandlePtr handle = DeviceManager::instance()->getHandleByMapping(mapping);
    if(handle.isNull() || handle->getFirmwareVersion() < 0)
    {
        answer["error"] = "Handle is null or device doesn't support Firmware informations.";
        Q_EMIT sendResult(cbID, answer);
        return;
    }

    int major = 0;
    int minor = 0;

    QString version =  versionInfo["version"].toString();
    QStringList split = version.split(".");
    if(split.count() > 1)
    {
        major = split[0].toInt();
        minor = split[1].toInt();
    }

    int versionInt = major * 1000 + minor;
    answer["info"] = versionInfo;
    if(handle->getFirmwareVersion() < versionInt)
    {
        answer["status"] = UPDATE_AVAILABLE;
        Q_EMIT sendResult(cbID, answer);
    }
    else
    {
        answer["status"] = UP_TO_DATE;
        Q_EMIT sendResult(cbID, answer);
    }
}
