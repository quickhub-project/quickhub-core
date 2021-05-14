/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef ENUMS_H
#define ENUMS_H
#include <QObject>

class Err : QObject
{
    Q_OBJECT

    /*!
        \enum Err::CloudError

        This enum type specifies the error codes for most of the operations.

        \value NO_ERROR
               All fine.
        \value UNKNOWN_TYPE
               The requested resource type doesn't exist.
        \value PERMISSION_DENIED
               The user has not the appropriate permissions.
        \value INVALID_TOKEN
               There is no session for the given token. LogIn and try again.
        \value ALREADY_EXISTS
               You tried to add a resource that already exists.
        \value INVALID_DESCRIPTOR
               The descriptor (resurce mapping is invalid).
        \value INVALID_DATA
               In most cases, this means you provided empty strings where it's not allowed or there are missing parameters.
    */

public:
    enum CloudError
    {
        NO_ERROR = 0,
        UNKNOWN_TYPE = -1,
        PERMISSION_DENIED = -2,
        INVALID_TOKEN = -3,
        ALREADY_EXISTS = -4,
        INVALID_DESCRIPTOR = -5,
        INVALID_DATA = -6
    };
    Q_ENUM (CloudError)

};

#endif // ENUMS_H
