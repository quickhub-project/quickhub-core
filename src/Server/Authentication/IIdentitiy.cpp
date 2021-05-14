/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "IIdentitiy.h"

IIdentity::IIdentity()
{


}
void IIdentity::setLastActivity(const qint64 &lastLoign)
{
    _lastActivity = lastLoign;
}


qint64 IIdentity::lastActivity() const
{
    return _lastActivity;
}
