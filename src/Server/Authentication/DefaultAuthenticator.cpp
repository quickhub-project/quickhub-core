/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "DefaultAuthenticator.h"
#include "../src/Storage/FileSystemLoader.h"

Q_GLOBAL_STATIC(DefaultAuthenticator, defaultAuthenticator);

iUserPtr DefaultAuthenticator::getUser(QString userID)
{
    QReadLocker locker(&_lock);
    return qSharedPointerCast<IUser>(_idToUserMap.value(userID, userPtr()));
}

DefaultAuthenticator::DefaultAuthenticator(QObject* parent) : IAuthenticator(parent)
{
}


void DefaultAuthenticator::init(QString userDataPath)
{
    if(_loader)
        return;

    _loader = new FileSystemLoader(userDataPath, this);
    _userDataPath = userDataPath;
    QVariantMap data = _loader->load();
    QVariantList users = data["users"].toList();
    QListIterator<QVariant> it(users);

    if(users.count() == 0)
    {
        userPtr admin(new User());
        QString password = "password";
        admin->setPassword(password);
        admin->setUserID("admin");
        admin->setUserPermission(ADD_USERS, true);
        admin->setUserPermission(DELETE_USERS, true);
        admin->setUserPermission(IS_ADMIN, true);
        admin->setUserPermission(MONITOR_USERS, true);
        admin->setUserPermission(MANAGE_DEVICES, true);
        addUser(admin);
    }

    while(it.hasNext())
    {
        userPtr user(new User(it.next().toMap(), this));
        addUser(user);
    }

    _saveTimer.setInterval(1000*60*5);
    _saveTimer.start();
    connect(&_saveTimer, &QTimer::timeout, this, &DefaultAuthenticator::save);
}


userPtr DefaultAuthenticator::getUserForID(QString userID)
{
    QReadLocker locker(&_lock);
    return  _idToUserMap.value(userID, QSharedPointer<User>());
}


userPtr DefaultAuthenticator::createUser(QString userID, QString password, AuthenticationService::ErrorCode* error)
{
    if(error)
        *error = AuthenticationService::NoError;

    if(userID.isEmpty() || password.isEmpty())
    {
        if(error)
            *error = AuthenticationService::InvalidData;
        return userPtr();
    }

    User* newUser = new User();
    newUser->setPassword(password);
    newUser->setUserID(userID);
    newUser->setUserName(userID);
    return  userPtr(newUser);
}



userPtr DefaultAuthenticator::addUser(QString userID, QString password, QString token, AuthenticationService::ErrorCode* error)
{
    userPtr user =  createUser(userID, password, error);
    if(!user.isNull())
        return addUser(user, token, error);
    return userPtr();
}

userPtr DefaultAuthenticator::addUser(userPtr user, QString token, AuthenticationService::ErrorCode* error)
{
    if(!_everybodyCanAddUsers)
    {
        iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
        if(user.isNull() || !user->isAuthorizedTo(ADD_USERS))
        {
            if(error)
                *error = AuthenticationService::PermissionDenied;
            return QSharedPointer<User>();
        }
    }

    QString userID = user->identityID();
    if(_idToUserMap.contains(userID))
    {
        if(error)
            *error = AuthenticationService::UserAlreadyExists;
        return QSharedPointer<User>();
    }

    if(error)
        *error = AuthenticationService::NoError;

    _users.append(user);
    save();
    _idToUserMap.insert(userID, user);
    Q_EMIT userAdded(user);
    return user;
}


QVector<userPtr> DefaultAuthenticator::getUsers(QString token, AuthenticationService::ErrorCode* error)
{
    if(token.isEmpty())
        return _users;

    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    QReadLocker locker(&_lock);
    if(user->isAuthorizedTo(MONITOR_USERS))
    {
        if(error)
            *error = AuthenticationService::NoError;

        return _users;
    }

    if(error)
        *error = AuthenticationService::PermissionDenied;
    return QVector<userPtr>();
}

AuthenticationService::ErrorCode DefaultAuthenticator::setPermission(QString token, QString userID, QString permission, bool allowed)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull() || !user->isAuthorizedTo(IS_ADMIN))
        return AuthenticationService::PermissionDenied;


    _lock.lockForRead();
    userPtr userToModify = _idToUserMap.value(userID, QSharedPointer<User>());
    _lock.unlock();

    if(userToModify.isNull())
        return AuthenticationService::InvalidData;
    userToModify->setUserPermission(permission, allowed);
    save();
    return AuthenticationService::NoError;
}


AuthenticationService::ErrorCode DefaultAuthenticator::changePassword(QString token, QString oldPassword, QString newPassword, QString userID)
{
    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);

    if(identity.isNull())
        return AuthenticationService::PermissionDenied;

    iUserPtr userToModify;
    if(!userID.isEmpty()) // Change password of other user
    {
        if(!identity->isAuthorizedTo(IS_ADMIN)) // can only be dony by admins
            return AuthenticationService::PermissionDenied;

        _lock.lockForRead();
        userToModify = _idToUserMap.value(userID, QSharedPointer<User>());
        _lock.unlock();
    }
    else
    {
        iUserPtr user = qSharedPointerCast<IUser>(identity);
        if(user.isNull())
            return AuthenticationService::InvalidData;

        if(!user->checkPassword(oldPassword)) // if you want to change your own password
            return AuthenticationService::IncorrectPassword;

        userToModify = user;
    }

    if (userToModify.isNull() || newPassword.isEmpty())
        return AuthenticationService::InvalidData;

    userToModify->setPassword(newPassword);
    save();
    return AuthenticationService::NoError;
}

const QVariantMap DefaultAuthenticator::getData()
{
    QVariantMap data;
    QVariantList users;
    _lock.lockForRead();
    QVectorIterator<userPtr> it(_users);
    while(it.hasNext())
    {
        users.append(it.next()->toVariant());
    }
    _lock.unlock();
    data["users"] = users;
    return data;
}

DefaultAuthenticator *DefaultAuthenticator::instance()
{
    return defaultAuthenticator;
}

void DefaultAuthenticator::addUser(userPtr user)
{
    _users.append(user);
    _idToUserMap.insert(user->identityID(), user);
}

void DefaultAuthenticator::save()
{
    _loader->save(getData());
}


void DefaultAuthenticator::deleteUser(userPtr user)
{
    auto tmpTokens = user->getTokens();
    QSetIterator<QString> it(tmpTokens);
    while(it.hasNext())
        AuthenticationService::instance()->logout(it.next());
    auto tmpSteadyTokens = user->getSteadyTokens();
    it = QSetIterator<QString>(tmpSteadyTokens);
    _lock.lockForWrite();
    _idToUserMap.remove(user->identityID());
    _users.removeAll(user);
    _lock.unlock();
    Q_EMIT userDeleted(user);
    save();
}


AuthenticationService::ErrorCode DefaultAuthenticator::deleteUser(QString token, QString password, QString userID)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
        return AuthenticationService::PermissionDenied;

    // check if user wants to delete itself
    if(userID.isEmpty() || userID == user->identityID())
    {
        userPtr castedUser = qSharedPointerCast<User>(user);
        if(castedUser.isNull())
            return AuthenticationService::InvalidData;

        if(castedUser->checkPassword(password))
        {
            deleteUser(castedUser);
            return AuthenticationService::NoError;
        }
    }

    _lock.lockForRead();
    iUserPtr userToDelete = _idToUserMap.value(userID, QSharedPointer<User>());
    _lock.unlock();


    if(!userToDelete.isNull())
    {
        if(!user->isAuthorizedTo(DELETE_USERS))
            return AuthenticationService::PermissionDenied;

        userPtr castedUserToDelete = qSharedPointerCast<User>(userToDelete);
        if(castedUserToDelete.isNull())
            return AuthenticationService::InvalidData;

        deleteUser(castedUserToDelete);
        return AuthenticationService::NoError;
    }

    return AuthenticationService::UserNotExists;
}
