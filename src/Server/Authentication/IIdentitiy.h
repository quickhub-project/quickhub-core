/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IAUTHENTICATABLE_H
#define IAUTHENTICATABLE_H

#include <QString>

class IIdentity
{

public:
    IIdentity();

    /*!
        \fn virtual bool IIdentity::isAuthorizedTo(QString permission) = 0;
        Returns true if the identity has the apropriate permission.
    */
    virtual bool isAuthorizedTo(QString permission) = 0;

    /*!
        \fn void IIdentity::setLastActivity(const qint64 &lastActivity)
        Is used by AuthenticationService. Stores the last activity timestamp.
    */
    void   setLastActivity(const qint64 &lastActivity);

    /*!
        \fn qint64 IIdentity::lastActivity() const;
        Returns the last activity timestamp
    */
    qint64 lastActivity() const;

    /*!
        \fn QString IIdentity::identityID() const = 0;
        Here a unique ID must be returned, which - in contrast to the login - must not change anymore.
        This ID is used to store identity related data in the database.
    */
    virtual QString identityID() const = 0;

    /*!
        \fn int IIdentity::sessionExpiration() const = 0;
        Returns the time (in seconds) that must elapse after the last use of a token before a session token
        becomes invalid.
    */
    virtual int sessionExpiration() const = 0;

    /*!
        \fn bool IIdentity::multipleSessionsAllowed() const = 0;
        Returns true if multiple sessions are allowed at the same time with this identity.
    */
    virtual bool multipleSessionsAllowed() const = 0;

    virtual void removeToken(QString token);
    virtual void addToken(QString token);


private:
    qint64          _lastActivity = -1;

};

#endif // IAUTHENTICATABLE_H
