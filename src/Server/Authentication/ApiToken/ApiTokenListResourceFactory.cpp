/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ApiTokenListResourceFactory.h"
#include "ApiTokenListResource.h"
#include "../AuthentificationService.h"

ApiTokenListResourceFactory::ApiTokenListResourceFactory(QObject* parent)
    : IResourceFactory(parent)
{
}

QString ApiTokenListResourceFactory::getResourceID(QString descriptor, QString token) const
{
    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    if (identity.isNull() || !identity->isAuthorizedTo(IS_ADMIN))
        return "";

    return descriptor;
}

QString ApiTokenListResourceFactory::getResourceType() const
{
    return "synclist";
}

QString ApiTokenListResourceFactory::getDescriptorPrefix() const
{
    return "apitokens";
}

resourcePtr ApiTokenListResourceFactory::createResource(QString token, QString descriptor, QObject* parent)
{
    Q_UNUSED(descriptor)
    Q_UNUSED(parent)

    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    if (identity.isNull() || !identity->isAuthorizedTo(IS_ADMIN))
        return nullptr;

    if (!_resource)
    {
        _resource = resourcePtr(new ApiTokenListResource());
    }

    return _resource;
}
