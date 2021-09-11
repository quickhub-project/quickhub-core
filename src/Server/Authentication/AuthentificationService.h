/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef AUTHENTIFICATIONSERVICE_H
#define AUTHENTIFICATIONSERVICE_H

#define ADD_USERS "addUsers"
#define DELETE_USERS "deleteUsers"
#define IS_ADMIN "isAdmin"
#define SERVICE "service"
#define MONITOR_USERS "monitorUsers"
#define MANAGE_DEVICES "manageDevices"
#define WRITE_DEVICES "writeDevices"
#define READ_DEVICES "readDevices"
#define SEE_DEVICES "seeDevices"

#include <QObject>
#include <QFile>
#include <QVector>
#include <QMap>
#include <QTimer>
#include <QSharedPointer>
#include <QReadWriteLock>
#include "qhcore_global.h"
#include "../Resources/ResourceManager/IResource.h"
//in seconds
#define SESSION_TIMEOUT 60*60

class IUser;
class IAuthenticator;
class IIdentity;
typedef QSharedPointer<IUser> iUserPtr;
typedef QSharedPointer<IIdentity> iIdentityPtr;

class COREPLUGINSHARED_EXPORT AuthenticationService : public QObject
{
    Q_OBJECT

public:
    explicit AuthenticationService(QObject *parent = nullptr);
    ~AuthenticationService() override;

    static AuthenticationService* instance();

    /*!
        \enum AuthenticationService::ErrorCode

        This enum type specifies the error codes for Authentification.

        \value NoError
               All fine.
        \value UserAlreadyExists
               If you want to add a user with a userID that already exists.
        \value IncompleteData
               There are missing parameters.
        \value InvalidData
               Means you provided empty strings in cases where it's not allowed.
        \value PermissionDenied
               The user has not the appropriate permissions.
        \value IncorrectPassword
               Invalid password.
        \value UserNotExists
               The user for the given userID does not exist.
    */

    enum ErrorCode
    {
        NoError = 0,
        UserAlreadyExists = -1,
        IncompleteData = - 2,
        InvalidData = -3,
        PermissionDenied = -4,
        IncorrectPassword = -5,
        UserNotExists = -6,
    };
    /*!
        \fn void AuthenticationService::registerAuthenticator(IAuthenticator* authenticator)
        This function allows to register an external authenticator-object. Subclass IAuthenticator and register its pointer with this function.
    */
    void registerAuthenticator(IAuthenticator* authenticator);

    /*!
        \fn void AuthenticationService::validateToken(QString token)
        This function checks if the session token exists and returns the apropriate user. The difference to AuthenticationService::getUserForToken(QString token)
        is that \c validateToken() will also renew the expiration timestamp and the last activity timestamp of the user.
        If there is no session with the given token, the function will return an invalid null pointer.
        \note Don't forget to check if the returned pointer is valid!
        \sa AuthenticationService::getUserForToken()
    */
    iIdentityPtr validateToken(QString token);

    iUserPtr validateUser(QString userID, QString password, ErrorCode* error = nullptr);

    /*!
        \fn void AuthenticationService::getUserForToken(QString token)
        This function checks if the session token exists and returns the apropriate user.This function will not renew the session / last access timestamps.
        Use AuthenticationService::validateToken() instead.
        If there is no session with the given token, the function will return an invalid null pointer.
        \note Don't forget to check if the returned pointer is valid!
        \sa AuthenticationService::validateToken()
    */
    iUserPtr getUserForToken(QString token) const;

    /*!
        \fn void AuthenticationService::getTokenExpiration(QString token)
        Returns a unix timestamp when the session token will expire.
    */
    qint64 getTokenExpiration(QString token);


    /*!
        \fn QString login(QString user, QString password, AuthenticationService::ErrorCode* error = 0);
        This function creates a session and returns the token.
    */
    QString login(QString user, QString password, AuthenticationService::ErrorCode* error = nullptr);

    QString login(iIdentityPtr identity, ErrorCode *error = nullptr);

    /*!
        \fn bool logout(QString token)
        Will remove the session and invalidate the token.
    */
    bool logout(QString token);


    iUserPtr getUserForUserID(QString userID);


signals:
    void sessionClosed(QString userID, QString token);


private:
    QList<IAuthenticator*> _authenticators;
    mutable QReadWriteLock _lock;
    QHash<QString, iIdentityPtr> _tokenToUserMap;
    QHash<QString, qint64> _tokenToExpiration;

};

#endif // AUTHENTIFICATIONSERVICE_H
