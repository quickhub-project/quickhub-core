/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef APITOKENMANAGER_H
#define APITOKENMANAGER_H

#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <QDateTime>
#include <QSharedPointer>
#include "qhcore_global.h"

class ApiTokenIdentity;
class IListResourceStorage;

struct CreateTokenResult
{
    QString token;
    QString uuid;
    QString name;
    bool success = false;
    QString errorMessage;
};

class COREPLUGINSHARED_EXPORT ApiTokenManager : public QObject
{
    Q_OBJECT

public:
    explicit ApiTokenManager(QObject* parent = nullptr);
    static ApiTokenManager* instance();

    CreateTokenResult createToken(const QString& name, const QString& description,
                                  const QVariantMap& permissions, const QDateTime& expirationDate);
    bool deleteToken(const QString& uuid);
    QVariantList getTokenMetadataList() const;
    void loadAndRegisterTokens();

signals:
    void tokenCreated(const QString& uuid);
    void tokenDeleted(const QString& uuid);

private slots:
    void checkExpiredTokens();

private:
    struct TokenRecord
    {
        QString uuid;
        QString name;
        QString description;
        QVariantMap permissions;
        QDateTime expirationDate;
        QDateTime createdAt;
        QString token;

        static TokenRecord fromVariant(const QVariantMap& map);
        QVariantMap toVariant() const;
    };

    QMap<QString, TokenRecord> _tokens;
    QMap<QString, QString> _uuidToToken;
    QTimer _expirationTimer;
    IListResourceStorage* _storage;
    static inline QString getRandomString(int count);
};

#endif // APITOKENMANAGER_H
