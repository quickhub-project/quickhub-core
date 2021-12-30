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
    // check every minute for timeouts
    _sessionTimeoutKicker.setInterval(60*1000);
    connect(&_sessionTimeoutKicker, &QTimer::timeout, this, &AuthenticationService::checkTimeouts);
    _sessionTimeoutKicker.start();
}

iIdentityPtr AuthenticationService::validateToken(QString token)
{
    _lock.lockForRead();
    iIdentityPtr user = _tokenToUserMap.value(token, QSharedPointer<User>());
    _lock.unlock();
    if(!user.isNull())
    {
        if(user->sessionExpiration() > 0)
        {
            _lock.lockForRead();
            qint64 tokenExpiration = _tokenToExpiration.value(token, 0);
            _lock.unlock();
            if(tokenExpiration > 0 && tokenExpiration < QDateTime::currentDateTime().toMSecsSinceEpoch())
            {
                qInfo()<< "Token expired. "<< user->identityID() <<" was forcibly logged out.";
                logout(token);
                return QSharedPointer<User>();
            }
            qint64 expiration = QDateTime::currentDateTime().addSecs(user->sessionExpiration()).toMSecsSinceEpoch();
            _lock.lockForWrite();
            _tokenToExpiration.insert(token, expiration);
            _lock.unlock();
        }

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


iUserPtr AuthenticationService::validateUser(QString userID, QString password, ErrorCode *error) const
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

        if(userObj->sessionExpiration() > 0)
        {
           qint64 expires = QDateTime::currentDateTime().addSecs(userObj->sessionExpiration()).toMSecsSinceEpoch();
           _lock.lockForWrite();
           _tokenToExpiration.insert(token, expires);
           _lock.unlock();
        }

        userObj->addToken(token);
        qInfo()<< userObj->identityID()+" logged in. ("<<userObj->sessionCount()<<" sessions open)";
        _lock.lockForWrite();
        _tokenToUserMap.insert(token, userObj);
        _lock.unlock();
    }

    return token;
}

QString AuthenticationService::login(iIdentityPtr identity, ErrorCode *error)
{
    if(!identity->multipleSessionsAllowed())
    {
        if (_tokenToUserMap.values().contains(identity))
        {
            if(error != nullptr)
                *error = PermissionDenied;
            return "";
        }
    }

    QString token = QUuid::createUuid().toString();
    if(identity->sessionExpiration() > 0)
    {
        qint64 expires = QDateTime::currentDateTime().addSecs(identity->sessionExpiration()).toMSecsSinceEpoch();
        _lock.lockForWrite();
        _tokenToExpiration.insert(token, expires);
        _lock.unlock();
    }

    qInfo()<< identity->identityID()+" logged in.";
    _lock.lockForWrite();
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
        identity->removeToken(token);

        qInfo()<< identity->identityID()+" logged out.";
        Q_EMIT sessionClosed(identity->identityID(), token);
        return true;
    }
    return false;
}

iUserPtr AuthenticationService::getUserForUserID(QString userID) const
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

void AuthenticationService::checkTimeouts()
{
    QHashIterator<QString, qint64> it(_tokenToExpiration);
    while(it.hasNext())
    {
        it.next();
        qint64 timeout = it.value();
        if(timeout > 0 && timeout < QDateTime::currentMSecsSinceEpoch())
        {
            qInfo()<< "Token expired. Identity forcibly logged out.";
            logout(it.key());
        }
    }
}




