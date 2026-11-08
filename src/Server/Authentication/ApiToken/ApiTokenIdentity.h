/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef APITOKENIDENTITY_H
#define APITOKENIDENTITY_H

#include <QObject>
#include <QDateTime>
#include <QVariantMap>
#include "../IIdentitiy.h"
#include "qhcore_global.h"

class COREPLUGINSHARED_EXPORT ApiTokenIdentity : public QObject, public IIdentity
{
    Q_OBJECT

public:
    ApiTokenIdentity(const QString& uuid, const QString& name,
                     const QString& description, const QVariantMap& permissions,
                     const QDateTime& expirationDate, QObject* parent = nullptr);

    // IIdentity
    bool isAuthorizedTo(QString permission) override;
    QString identityID() const override;
    int sessionExpiration() const override;
    bool multipleSessionsAllowed() const override;

    // Accessors
    QString name() const;
    QString description() const;
    QVariantMap permissions() const;
    QDateTime expirationDate() const;
    QString uuid() const;

private:
    QString _uuid;
    QString _name;
    QString _description;
    QVariantMap _permissions;
    QDateTime _expirationDate;
};

#endif // APITOKENIDENTITY_H
