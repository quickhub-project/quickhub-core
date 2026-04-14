/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "AuthentificationService.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QUuid>
#include <QVariant>
#include <QVector>
#include "User.h"
#include "IAuthenticator.h"

// ----------------------------------------------------------------------------
// Logging
// ----------------------------------------------------------------------------

Q_LOGGING_CATEGORY(lcAuthService, "quickhub.authservice")

// ----------------------------------------------------------------------------
// Singleton
// ----------------------------------------------------------------------------

Q_GLOBAL_STATIC(AuthenticationService, authenticationService);

AuthenticationService::~AuthenticationService() = default;

AuthenticationService *AuthenticationService::instance()
{
    return authenticationService;
}


// ----------------------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------------------

AuthenticationService::AuthenticationService(QObject *parent) : QObject(parent),
    _lock(QReadWriteLock::Recursive)
{
    // Check every minute for expired sessions.
    _sessionTimeoutKicker.setInterval(60*1000);
    connect(&_sessionTimeoutKicker, &QTimer::timeout, this, &AuthenticationService::checkTimeouts);
    _sessionTimeoutKicker.start();
}

// ----------------------------------------------------------------------------
// Authenticator registration
// ----------------------------------------------------------------------------

void AuthenticationService::registerAuthenticator(IAuthenticator *authenticator)
{
    _lock.lockForWrite();
    _authenticators << authenticator;
    _lock.unlock();
    qCInfo(lcAuthService) << "Authenticator registered:" << authenticator->metaObject()->className();
}

// ----------------------------------------------------------------------------
// Token validation
// ----------------------------------------------------------------------------

iIdentityPtr AuthenticationService::validateToken(QString token)
{
    _lock.lockForWrite();
    iIdentityPtr identitiy = _tokenToUserMap.value(token, QSharedPointer<User>());
    if(!identitiy.isNull())
    {
        if(identitiy->sessionExpiration() > 0)
        {
            qint64 tokenExpiration = _tokenToExpiration.value(token, 0);
            if(tokenExpiration > 0 && tokenExpiration < QDateTime::currentDateTime().toMSecsSinceEpoch())
            {
                qCInfo(lcAuthService) << "Token expired, identity =" << identitiy->identityID() << "- forcibly logged out";
                _lock.unlock();
                logout(token);
                return QSharedPointer<User>();
            }
            qint64 expiration = QDateTime::currentDateTime().addSecs(identitiy->sessionExpiration()).toMSecsSinceEpoch();
            _tokenToExpiration.insert(token, expiration);
        }

        identitiy->setLastActivity(QDateTime::currentMSecsSinceEpoch());
    }
    else
    {
        qCDebug(lcAuthService) << "validateToken: unknown token";
    }
    _lock.unlock();
    return identitiy;
}

bool AuthenticationService::isValidToken(QString token)
{
    bool tokenIsValid = false;
    _lock.lockForWrite();
    {
        iIdentityPtr identitiy = _tokenToUserMap.value(token, QSharedPointer<User>());
        if(!identitiy.isNull())
        {
            if(identitiy->sessionExpiration() > 0)
            {
                qint64 tokenExpiration = _tokenToExpiration.value(token, 0);
                if(tokenExpiration > 0 && tokenExpiration < QDateTime::currentDateTime().toMSecsSinceEpoch())
                {
                    qCInfo(lcAuthService) << "Token expired, identity =" << identitiy->identityID() << "- forcibly logged out";
                    QMetaObject::invokeMethod(this, "logout", Qt::QueuedConnection,
                                              Q_ARG(QString, token));
                }
                else
                {
                    qint64 expiration = QDateTime::currentDateTime().addSecs(identitiy->sessionExpiration()).toMSecsSinceEpoch();
                        _tokenToExpiration.insert(token, expiration);
                        tokenIsValid = true;
                }
            }
            else
            {
                tokenIsValid = true;
            }
        }

        if(tokenIsValid)
        {
            identitiy->setLastActivity(QDateTime::currentMSecsSinceEpoch());
        }
    }
    _lock.unlock();
    return tokenIsValid;
}



// ----------------------------------------------------------------------------
// Public API
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
// User validation
// ----------------------------------------------------------------------------

iUserPtr AuthenticationService::validateUser(QString userID, QString password, ErrorCode *error) const
{
    QReadLocker locker(&_lock);
    return validateUser_locked(userID, password, error);
}

iUserPtr AuthenticationService::validateUser_locked(QString userID, QString password, ErrorCode *error) const
{
    AuthenticationService::ErrorCode returnError = UnknownInternalError;

    iUserPtr userObj = getUserForUserID_locked(userID);

    if(!userObj.isNull())
    {
        switch(userObj->checkPassword(password))
        {
            case IUser::CheckPasswortResult::PASSWORD_OK:
            {
                returnError = NoError;
                break;
            }

            case IUser::CheckPasswortResult::PASSWORD_WRONG:
            {
                qCWarning(lcAuthService) << "Login failed (incorrect password), userID =" << userID;
                returnError = IncorrectPassword;
                break;
            }

            case IUser::CheckPasswortResult::PASSSWORD_RESET_REQUESTED:
            {
                returnError = PasswordResetRequested;
                break;
            }
        }

    }
    else
    {
        qCWarning(lcAuthService) << "Login failed (user not found), userID =" << userID;
        returnError =  UserNotExists;
    }

    if(error)
        *error = returnError;

    return userObj;
}

// ----------------------------------------------------------------------------
// Login / Logout
// ----------------------------------------------------------------------------

QString AuthenticationService::login(QString userID, QString password, ErrorCode *error)
{
    _lock.lockForWrite();
    iUserPtr userObj = validateUser_locked(userID, password, error);
    QString token = "";

    if(!userObj.isNull() && (*error == NoError || *error == PasswordResetRequested))
    {
        if(userObj->isAuthorizedTo(SERVICE) && userObj->sessionCount()  >= 1)
        {
            qCWarning(lcAuthService) << "Login denied (service account limit), userID =" << userID;
            _lock.unlock();
            *error = PermissionDenied;
            return "";
        }

        token = QUuid::createUuid().toString();

        if(userObj->sessionExpiration() > 0)
        {
           qint64 expires = QDateTime::currentDateTime().addSecs(userObj->sessionExpiration()).toMSecsSinceEpoch();
           _tokenToExpiration.insert(token, expires);
        }

        userObj->addToken(token);
        _tokenToUserMap.insert(token, userObj);
    }
    _lock.unlock();

    if(!userObj.isNull() && *error == NoError)
        qCInfo(lcAuthService) << "Logged in:" << userObj->identityID() << "(" << userObj->sessionCount() << "sessions open)";

    return token;
}

QString AuthenticationService::login(iIdentityPtr identity, ErrorCode *error)
{
    if(!identity->multipleSessionsAllowed())
    {
        QReadLocker locker(&_lock);
        if (_tokenToUserMap.values().contains(identity))
        {
            qCWarning(lcAuthService) << "Login denied (multiple sessions not allowed), identity =" << identity->identityID();
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

    qCInfo(lcAuthService) << "Logged in:" << identity->identityID();
    _lock.lockForWrite();
    _tokenToUserMap.insert(token, identity);
    _lock.unlock();
    return token;
}

// ----------------------------------------------------------------------------
// Token registration
// ----------------------------------------------------------------------------

AuthenticationService::ErrorCode AuthenticationService::registerToken(QString token, iIdentityPtr identity)
{
    if (!identity->multipleSessionsAllowed())
    {
        QReadLocker locker(&_lock);
        if (_tokenToUserMap.values().contains(identity))
        {
            qCWarning(lcAuthService) << "registerToken denied (multiple sessions not allowed), identity =" << identity->identityID();
            return PermissionDenied;
        }
    }

    if (identity->sessionExpiration() > 0)
    {
        qint64 expires = QDateTime::currentDateTime().addSecs(identity->sessionExpiration()).toMSecsSinceEpoch();
        _lock.lockForWrite();
        _tokenToExpiration.insert(token, expires);
        _lock.unlock();
    }

    qCInfo(lcAuthService) << "Token registered, identity =" << identity->identityID();
    _lock.lockForWrite();
    _tokenToUserMap.insert(token, identity);
    _lock.unlock();
    return NoError;
}

bool AuthenticationService::logout(QString token)
{
    _lock.lockForWrite();
    iIdentityPtr identity = _tokenToUserMap.value(token, QSharedPointer<IIdentity>());
    if(!identity.isNull())
    {
        _tokenToUserMap.remove(token);
        _tokenToExpiration.remove(token);
        identity->removeToken(token);
        _lock.unlock();

        qCInfo(lcAuthService) << "Logged out:" << identity->identityID();
        Q_EMIT sessionClosed(identity->identityID(), token);
        return true;
    }
    qCDebug(lcAuthService) << "logout: unknown token";
    _lock.unlock();
    return false;
}

// ----------------------------------------------------------------------------
// User lookup
// ----------------------------------------------------------------------------

iUserPtr AuthenticationService::getUserForUserID(QString userID) const
{
    QReadLocker locker(&_lock);
    return getUserForUserID_locked(userID);
}

iUserPtr AuthenticationService::getUserForUserID_locked(QString userID) const
{
    QListIterator<QPointer<IAuthenticator>> it(_authenticators);
    iUserPtr userObj;

    while(it.hasNext())
    {
        auto authenticator = it.next();
        if(authenticator.isNull()){
            continue;
        }
        userObj = authenticator->getUser(userID);
        if(!userObj.isNull())
        {
            break;
        }
    }

    return userObj;
}

bool AuthenticationService::alreadyExists(QString userID) const
{
    QReadLocker locker(&_lock);
    QListIterator<QPointer<IAuthenticator>> it(_authenticators);

    while(it.hasNext())
    {
        auto authenticator = it.next();
        if(authenticator.isNull()){
            continue;
        }
        if(!authenticator->isUnusedUserID(userID))
            return true;
    }

    return false;
}

// ----------------------------------------------------------------------------
// Session timeout
// ----------------------------------------------------------------------------

void AuthenticationService::checkTimeouts()
{
    QStringList expiredTokens;
    {
        QReadLocker locker(&_lock);
        QHashIterator<QString, qint64> it(_tokenToExpiration);
        while(it.hasNext())
        {
            it.next();
            qint64 timeout = it.value();
            if(timeout > 0 && timeout < QDateTime::currentMSecsSinceEpoch())
            {
                expiredTokens.append(it.key());
            }
        }
    }

    for(const QString& token : expiredTokens)
    {
        qCInfo(lcAuthService) << "Session timeout, forcibly logging out token";
        logout(token);
    }
}
