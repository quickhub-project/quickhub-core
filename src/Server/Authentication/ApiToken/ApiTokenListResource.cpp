/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ApiTokenListResource.h"
#include "ApiTokenManager.h"
#include "../AuthentificationService.h"

ApiTokenListResource::ApiTokenListResource(QObject* parent)
    : ListResource(nullptr, parent)
{
    setAllowUserAccess(false);
    connect(ApiTokenManager::instance(), &ApiTokenManager::tokenCreated, this, &ApiTokenListResource::onTokenCreated);
    connect(ApiTokenManager::instance(), &ApiTokenManager::tokenDeleted, this, &ApiTokenListResource::onTokenDeleted);
}

QVariantList ApiTokenListResource::getListData() const
{
    return ApiTokenManager::instance()->getTokenMetadataList();
}

int ApiTokenListResource::getCount() const
{
    return ApiTokenManager::instance()->getTokenMetadataList().count();
}

IResource::ModificationResult ApiTokenListResource::appendItem(QVariant data, QString token)
{
    Q_UNUSED(data)
    Q_UNUSED(token)
    ModificationResult result;
    result.error = PERMISSION_DENIED;
    return result;
}

IResource::ModificationResult ApiTokenListResource::removeItem(QString uuid, QString token, int index)
{
    Q_UNUSED(index)
    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);

    if (identity.isNull() || !identity->isAuthorizedTo(IS_ADMIN))
    {
        ModificationResult result;
        result.error = PERMISSION_DENIED;
        return result;
    }

    ModificationResult result;
    if (!ApiTokenManager::instance()->deleteToken(uuid))
    {
        result.error = UNKNOWN_ITEM;
    }
    return result;
}

bool ApiTokenListResource::isPermittedToRead(iIdentityPtr identity) const
{
    return !identity.isNull() && identity->isAuthorizedTo(IS_ADMIN);
}

bool ApiTokenListResource::isPermittedToWrite(iIdentityPtr identity) const
{
    return !identity.isNull() && identity->isAuthorizedTo(IS_ADMIN);
}

void ApiTokenListResource::onTokenCreated(const QString& uuid)
{
    Q_UNUSED(uuid)
    Q_EMIT reset();
}

void ApiTokenListResource::onTokenDeleted(const QString& uuid)
{
    Q_UNUSED(uuid)
    Q_EMIT reset();
}
