/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ApiTokenIdentity.h"

ApiTokenIdentity::ApiTokenIdentity(const QString& uuid, const QString& name,
                                   const QString& description, const QVariantMap& permissions,
                                   const QDateTime& expirationDate, QObject* parent)
    : QObject(parent)
    , _uuid(uuid)
    , _name(name)
    , _description(description)
    , _permissions(permissions)
    , _expirationDate(expirationDate)
{
}

bool ApiTokenIdentity::isAuthorizedTo(QString permission)
{
    return _permissions.value(permission, false).toBool();
}

QString ApiTokenIdentity::identityID() const
{
    return "apitoken:" + _uuid;
}

int ApiTokenIdentity::sessionExpiration() const
{
    return -1;
}

bool ApiTokenIdentity::multipleSessionsAllowed() const
{
    return true;
}

QString ApiTokenIdentity::name() const
{
    return _name;
}

QString ApiTokenIdentity::description() const
{
    return _description;
}

QVariantMap ApiTokenIdentity::permissions() const
{
    return _permissions;
}

QDateTime ApiTokenIdentity::expirationDate() const
{
    return _expirationDate;
}

QString ApiTokenIdentity::uuid() const
{
    return _uuid;
}
