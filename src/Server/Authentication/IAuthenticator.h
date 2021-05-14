/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IAUTHENTICATOR_H
#define IAUTHENTICATOR_H
#include <QObject>
#include "AuthentificationService.h"

class IAuthenticator : public QObject
{
    Q_OBJECT
    /*!
        \class IAuthenticator
        \brief This class is the interface you need to implement when you want to integrate your own User-Database.

        Make sure to register yout Authenticator instance with  AuthenticationService::registerAuthenticator

    */
public:
    IAuthenticator(QObject* parent = nullptr) : QObject(parent){}
    virtual ~IAuthenticator(){}
    /*!
        Search the apropriate user in your database, create (custom) User object and return a shared pointer to it.
        Return a nullpointer when the requested user doesn`t exists.
    */
    virtual iUserPtr getUser(QString userID) = 0;
};

#endif // IAUTHENTICATOR_H
