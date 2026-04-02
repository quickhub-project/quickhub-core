/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ApiTokenService.h"
#include "ApiTokenManager.h"
#include "../AuthentificationService.h"
#include "../IIdentitiy.h"
#include <QDateTime>
#include <QDebug>

ApiTokenService::ApiTokenService(QObject* parent)
    : IService(parent)
{
}

QString ApiTokenService::getServiceName() const
{
    return "apitokens";
}

QStringList ApiTokenService::getServiceCalls() const
{
    return QStringList() << "addToken";
}

bool ApiTokenService::call(QString call, QString token, QString cbID, QVariant argument)
{
    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);

    if (identity.isNull() || !identity->isAuthorizedTo(IS_ADMIN))
    {
        QVariantMap answer;
        answer["errorcode"] = -1;
        answer["errorstring"] = "Permission denied";
        Q_EMIT response(cbID, answer);
        return true;
    }

    if (call == "addToken")
    {
        QVariantMap args = argument.toMap();
        QString name = args.value("name").toString();
        QString description = args.value("description").toString();
        QVariantMap permissions = args.value("permissions").toMap();
        QDateTime expirationDate;

        if (args.contains("expirationDate") && !args.value("expirationDate").toString().isEmpty())
            expirationDate = QDateTime::fromString(args.value("expirationDate").toString(), Qt::ISODate);

        CreateTokenResult result = ApiTokenManager::instance()->createToken(name, description, permissions, expirationDate);

        QVariantMap answer;
        if (result.success)
        {
            answer["token"] = result.token;
            answer["uuid"] = result.uuid;
            answer["name"] = result.name;
            answer["errorcode"] = 0;
        }
        else
        {
            answer["errorcode"] = -1;
            answer["errorstring"] = result.errorMessage;
        }

        Q_EMIT response(cbID, answer);
        return true;
    }

    if (call == "deleteToken")
    {
        QVariantMap args = argument.toMap();
        QString uuid = args.value("uuid").toString();


        QVariantMap answer;
        if(ApiTokenManager::instance()->deleteToken(uuid)){
            answer["errorcode"] = 0;
        }
        else{
            answer["errorcode"] = -1;
        }

        Q_EMIT response(cbID, answer);
        return true;
    }
    return false;
}
