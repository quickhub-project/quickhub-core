/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "IList.h"
#include "Server/Authentication/AuthentificationService.h"


IList::IList(QObject *parent) : QObject(parent)
{
}

bool IList::isPermitted(QString token) const
{
    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);
    return !identity.isNull();
}

void IList::handleMessage(QVariant msg, ISocket *handle)
{
    Q_UNUSED(msg)
    Q_UNUSED(handle)
}
