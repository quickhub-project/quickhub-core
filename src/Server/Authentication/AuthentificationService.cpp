/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "AuthentificationService.h"
#include <QFileInfo>
#include <QVariant>
#include <QJsonDocument>
#include <QDebug>
#include <QDir>
#include <QVector>
#include <QUuid>
#include <QDateTime>
#include <QCoreApplication>
#include "User.h"
#include "IAuthenticator.h"

Q_GLOBAL_STATIC(AuthenticationService, authenticationService);

AuthenticationService::~AuthenticationService()
{
}

AuthenticationService *AuthenticationService::instance()
{
    return authenticationService;
}


void AuthenticationService::registerAuthenticator(IAuthenticator *authenticator)
{
    _lock.lockForWrite();
    _authenticators << authenticator;
    _lock.unlock();
}



AuthenticationService::AuthenticationService(QObject *parent) : QObject(parent),
    _lock(QReadWriteLock::Recursive)
{
}

iIdentityPtr AuthenticationService::validateToken(QString token)
{
    _lock.lockForRead();
    iIdentityPtr user = _tokenToUserMap.value(token, QSharedPointer<User>());
    _lock.unlock();
    if(!user.isNull())
    {
        _lock.lockForWrite();
        _tokenToExpiration.insert(token,QDateTime::currentDateTime().addSecs(SESSION_TIMEOUT).toMSecsSinceEpoch());
        _lock.unlock();

        user->setLastActivity(QDateTime::currentMSecsSinceEpoch());
    }
    return user;
}



iUserPtr AuthenticationService::getUserForToken(QString token) const
{
    QReadLocker locker(&_lock);
    iIdentityPtr identity = _tokenToUserMap.value(token, QSharedPointer<IIdentity>());
    return qSharedPointerCast<IUser>(identity);
}



qint64 AuthenticationService::getTokenExpiration(QString token)
{
    QReadLocker locker(&_lock);
    return _tokenToExpiration.value(token, 0);
}


iUserPtr AuthenticationService::validateUser(QString userID, QString password, ErrorCode *error)
{

    AuthenticationService::ErrorCode returnError = NoError;

    iUserPtr userObj = getUserForUserID(userID);

    if(!userObj.isNull())
    {
        if(!userObj->checkPassword(password))
        {
            returnError = IncorrectPassword;
        }
    }
    else
    {
        returnError =  UserNotExists;
    }

    if(error)
        *error = returnError;

    return userObj;
}

QString AuthenticationService::login(QString userID, QString password, ErrorCode *error)
{
    iUserPtr userObj = validateUser(userID, password, error);
    QString token = "";

    if(!userObj.isNull() && *error == NoError)
    {

        if(userObj->isAuthorizedTo(SERVICE) && userObj->sessionCount()  >= 1)
        {
            *error = PermissionDenied;
            return "";
        }

        token = QUuid::createUuid().toString();
        qint64 expires;

        if(SESSION_TIMEOUT < 0)
            expires = -1;
        else
            expires = QDateTime::currentDateTime().addSecs(SESSION_TIMEOUT).toMSecsSinceEpoch();

        userObj->addToken(token);
        qInfo()<< userObj->identityID()+" logged in. ("<<userObj->sessionCount()<<" sessions open)";
        _lock.lockForWrite();
        _tokenToExpiration.insert(token, expires);
        _tokenToUserMap.insert(token, userObj);
        _lock.unlock();
    }

    return token;
}

QString AuthenticationService::login(iIdentityPtr identity, ErrorCode *error)
{
    if (_tokenToUserMap.values().contains(identity))
    {
        *error = PermissionDenied;
        return "";
    }

    QString token = QUuid::createUuid().toString();
    qint64 expires;

    if(SESSION_TIMEOUT < 0)
        expires = -1;
    else
        expires = QDateTime::currentDateTime().addSecs(SESSION_TIMEOUT).toMSecsSinceEpoch();

    qInfo()<< identity->identityID()+" logged in.";
    _lock.lockForWrite();
    _tokenToExpiration.insert(token, expires);
    _tokenToUserMap.insert(token, identity);
    _lock.unlock();
    return token;
}



bool AuthenticationService::logout(QString token)
{
    _lock.lockForRead();
    iIdentityPtr identity = _tokenToUserMap.value(token, QSharedPointer<IIdentity>());
    _lock.unlock();
    if(!identity.isNull())
    {
        _lock.lockForWrite();
        _tokenToUserMap.remove(token);
        _tokenToExpiration.remove(token);
        _lock.unlock();

        iUserPtr user = qSharedPointerCast<IUser>(identity);
        if(!user.isNull())
            user->removeToken(token);

        qInfo()<< user->identityID()+" logged out. (still "<<user->sessionCount()<<" sessions open)";
        Q_EMIT sessionClosed(user->identityID(), token);
        return true;
    }
    return false;
}

iUserPtr AuthenticationService::getUserForUserID(QString userID)
{
    _lock.lockForRead();
    QListIterator<IAuthenticator*> it(_authenticators);
    _lock.unlock();
    iUserPtr userObj;

    while(it.hasNext())
    {
        userObj = it.next()->getUser(userID);
        if(!userObj.isNull())
            break;
    }

    return userObj;
}




