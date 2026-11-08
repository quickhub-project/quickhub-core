/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ApiTokenManager.h"
#include "ApiTokenIdentity.h"
#include "../AuthentificationService.h"
#include "../../../Storage/ListResourceFileSystemStorage.h"

#include <QRandomGenerator>
#include <QUuid>
#include <QLoggingCategory>
#include <QGlobalStatic>

Q_LOGGING_CATEGORY(lcApiTokenManager, "quickhub.auth.apitoken")

Q_GLOBAL_STATIC(ApiTokenManager, apiTokenManager)

// ----------------------------------------------------------------------------
// Ctors
// ----------------------------------------------------------------------------

/*!
    Constructs the ApiTokenManager. Initializes the filesystem storage for
    persisted token records and starts a periodic timer that checks for
    expired tokens every 60 seconds.
*/
ApiTokenManager::ApiTokenManager(QObject* parent)
    : QObject(parent),
      _storage(new ListResourceFileSystemStorage("apitokens", this))
{
    _expirationTimer.setInterval(60 * 1000);
    connect(&_expirationTimer, &QTimer::timeout, this, &ApiTokenManager::checkExpiredTokens);
    _expirationTimer.start();
}

ApiTokenManager* ApiTokenManager::instance()
{
    return apiTokenManager;
}

// ----------------------------------------------------------------------------
// TokenRecord serialization
// ----------------------------------------------------------------------------

ApiTokenManager::TokenRecord ApiTokenManager::TokenRecord::fromVariant(const QVariantMap& map)
{
    TokenRecord record;
    record.uuid = map.value("uuid").toString();
    record.name = map.value("name").toString();
    record.description = map.value("description").toString();
    record.permissions = map.value("permissions").toMap();
    record.createdAt = QDateTime::fromString(map.value("createdAt").toString(), Qt::ISODate);
    record.token = map.value("token").toString();

    QString expStr = map.value("expirationDate").toString();
    if (!expStr.isEmpty())
        record.expirationDate = QDateTime::fromString(expStr, Qt::ISODate);

    return record;
}

QVariantMap ApiTokenManager::TokenRecord::toVariant() const
{
    QVariantMap map;
    map["uuid"] = uuid;
    map["name"] = name;
    map["description"] = description;
    map["permissions"] = permissions;
    map["createdAt"] = createdAt.toString(Qt::ISODate);
    map["token"] = token;
    if (expirationDate.isValid())
        map["expirationDate"] = expirationDate.toString(Qt::ISODate);
    return map;
}

// ----------------------------------------------------------------------------
// Token lifecycle
// ----------------------------------------------------------------------------

/*!
    Creates a new API token with the given \a name, \a description,
    \a permissions and optional \a expirationDate. The token is registered
    with the AuthenticationService so it can be used for immediate
    authentication. Returns a CreateTokenResult indicating success or failure.
*/
CreateTokenResult ApiTokenManager::createToken(const QString& name, const QString& description,
                                               const QVariantMap& permissions, const QDateTime& expirationDate)
{
    CreateTokenResult result;

    if (name.isEmpty())
    {
        qCWarning(lcApiTokenManager) << "Token creation failed: name is empty.";
        result.errorMessage = "Token name must not be empty";
        return result;
    }

    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString token = getRandomString(128);

    // Create an identity that represents the API token's permissions.
    auto* identity = new ApiTokenIdentity(uuid, name, description, permissions, expirationDate);
    auto identityPtr = QSharedPointer<IIdentity>(identity);

    AuthenticationService::ErrorCode error = AuthenticationService::instance()->registerToken(token, identityPtr);

    if (error != AuthenticationService::NoError)
    {
        qCWarning(lcApiTokenManager) << "Token creation failed: AuthenticationService rejected token."
                                     << "name:" << name
                                     << "error:" << error;
        result.errorMessage = "Failed to register token";
        return result;
    }

    TokenRecord record;
    record.uuid = uuid;
    record.name = name;
    record.description = description;
    record.permissions = permissions;
    record.expirationDate = expirationDate;
    record.createdAt = QDateTime::currentDateTimeUtc();
    record.token = token;

    _tokens.insert(uuid, record);
    _uuidToToken.insert(uuid, token);

    // Persist the token record to filesystem storage.
    _storage->appendItem(record.toVariant());

    result.token = token;
    result.uuid = uuid;
    result.name = name;
    result.success = true;

    qCInfo(lcApiTokenManager) << "Token created."
                              << "name:" << name
                              << "uuid:" << uuid;
    Q_EMIT tokenCreated(uuid);
    return result;
}

/*!
    Deletes the API token identified by \a uuid. The token is logged out
    from the AuthenticationService, removed from the in-memory maps and
    deleted from persistent storage. Returns \c true if the token was found
    and deleted, \c false if no token with that UUID exists.
*/
bool ApiTokenManager::deleteToken(const QString& uuid)
{
    if (!_tokens.contains(uuid))
    {
        qCDebug(lcApiTokenManager) << "Delete requested for unknown token."
                                   << "uuid:" << uuid;
        return false;
    }

    // Invalidate the token's active session.
    QString token = _uuidToToken.value(uuid);
    if (!token.isEmpty())
    {
        AuthenticationService::instance()->logout(token);
    }

    _tokens.remove(uuid);
    _uuidToToken.remove(uuid);

    _storage->removeItem({uuid, -1});

    qCInfo(lcApiTokenManager) << "Token deleted."
                              << "uuid:" << uuid;
    Q_EMIT tokenDeleted(uuid);

    return true;
}

// ----------------------------------------------------------------------------
// Query
// ----------------------------------------------------------------------------

/*!
    Returns a list of metadata entries for all currently active API tokens.
    Each entry is a QVariantMap with \c uuid, \c timestamp and a nested
    \c data map containing \c name, \c description, \c permissions,
    \c expirationDate and \c createdAt. The token secret itself is never
    included in the metadata.
*/
QVariantList ApiTokenManager::getTokenMetadataList() const
{
    QVariantList list;
    for (auto it = _tokens.constBegin(); it != _tokens.constEnd(); ++it)
    {
        const TokenRecord& record = it.value();
        QVariantMap data = record.toVariant();
        data.remove("uuid");
        data.remove("token");

        QVariantMap item;
        item["uuid"] = record.uuid;
        item["timestamp"] = record.createdAt.toMSecsSinceEpoch();
        item["data"] = data;
        list.append(item);
    }
    return list;
}

// ----------------------------------------------------------------------------
// Persistence
// ----------------------------------------------------------------------------

/*!
    Loads all token records from persistent storage and registers them with
    the AuthenticationService. Tokens that are already expired or have
    invalid data (missing uuid or token) are removed from storage and
    skipped. Should be called once during server startup.
*/
void ApiTokenManager::loadAndRegisterTokens()
{
    QVariantList items = _storage->getList();
    QDateTime now = QDateTime::currentDateTimeUtc();

    qCInfo(lcApiTokenManager) << "Loading persisted tokens."
                              << "count:" << items.count();

    // Iterate in reverse so that removing items by index doesn't invalidate
    // subsequent indices.
    for (int i = items.count() - 1; i >= 0; --i)
    {
        TokenRecord record = TokenRecord::fromVariant(items.at(i).toMap());

        if (record.uuid.isEmpty() || record.token.isEmpty())
        {
            qCWarning(lcApiTokenManager) << "Skipping invalid token record (missing uuid or token)."
                                         << "index:" << i;
            _storage->removeItem({record.uuid, i});
            continue;
        }

        if (record.expirationDate.isValid() && record.expirationDate < now)
        {
            qCInfo(lcApiTokenManager) << "Skipping expired token, removing from storage."
                                      << "name:" << record.name
                                      << "uuid:" << record.uuid;
            _storage->removeItem({record.uuid, i});
            continue;
        }

        auto* identity = new ApiTokenIdentity(record.uuid, record.name, record.description,
                                              record.permissions, record.expirationDate);
        auto identityPtr = QSharedPointer<IIdentity>(identity);

        AuthenticationService::ErrorCode error = AuthenticationService::instance()->registerToken(record.token, identityPtr);

        if (error != AuthenticationService::NoError)
        {
            qCWarning(lcApiTokenManager) << "Failed to register token with AuthenticationService."
                                         << "name:" << record.name
                                         << "error:" << error;
            _storage->removeItem({record.uuid, i});
            continue;
        }

        _tokens.insert(record.uuid, record);
        _uuidToToken.insert(record.uuid, record.token);

        qCDebug(lcApiTokenManager) << "Token loaded."
                                   << "name:" << record.name
                                   << "uuid:" << record.uuid;
    }

    qCInfo(lcApiTokenManager) << "Token loading complete."
                              << "active:" << _tokens.count();
}


// ----------------------------------------------------------------------------
// Expiration
// ----------------------------------------------------------------------------

/*!
    Periodic slot called by \c _expirationTimer. Scans all active tokens
    and deletes any whose expiration date has passed.
*/
void ApiTokenManager::checkExpiredTokens()
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    QStringList expiredUuids;

    for (auto it = _tokens.constBegin(); it != _tokens.constEnd(); ++it)
    {
        const TokenRecord& record = it.value();
        if (record.expirationDate.isValid() && record.expirationDate < now)
        {
            expiredUuids.append(record.uuid);
        }
    }

    if (!expiredUuids.isEmpty())
    {
        qCInfo(lcApiTokenManager) << "Expired tokens found."
                                  << "count:" << expiredUuids.count();
    }

    for (const QString& uuid : expiredUuids)
    {
        qCInfo(lcApiTokenManager) << "Removing expired token."
                                  << "name:" << _tokens.value(uuid).name
                                  << "uuid:" << uuid;
        deleteToken(uuid);
    }
}

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------
QString ApiTokenManager::getRandomString(int count)
{
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwxyzZ0123456789-.!?*§");
    QString randomString;
    for(int i=0; i < count; ++i)
    {
        int index = QRandomGenerator::global()->bounded(possibleCharacters.length());
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}

