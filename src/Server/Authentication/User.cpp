/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "User.h"
#include <QVariant>
#include <QDebug>


User::User(QObject *parent) : IUser(parent)
{

}

User::User(QVariantMap variant, QObject *parent) : IUser(parent)
{
    _group = variant["group"].toString();
    _eMail = variant["eMail"].toString();
    _userName = variant["userName"].toString();
    _passHash = variant["passHash"].toString();
    _userID = variant["userID"].toString();
    _userPermissions = variant["userPermissions"].toMap();
    _userData = variant["userData"].toMap();
    setLastActivity(variant["lastActivity"].toString().toLongLong());
    QVariantList tokens = variant["steadyTokens"].toList();
    QListIterator<QVariant> it(tokens);
    while(it.hasNext())
    {
        _steadyTokens.insert(it.next().toString());
    }
}

User::~User()
{
    qDebug()<<"User "+_userID+" deleted. RIP.";
}

QVariantMap User::toVariant() const
{
    QVariantMap user;
    user["group"] = group();
    user["eMail"] = getEMail();
    user["userName"] = getUserName();
    user["passHash"] = passHash();
    user["userID"] = identityID();
    user["userPermissions"] = userPermissions();
    user["lastActivity"] = lastActivity();
    user["userData"]  = getUserData();

    QVariantList tokens;
    QSetIterator<QString> it(getSteadyTokens());
    while(it.hasNext())
    {
        tokens << it.next();
    }
    user["steadyTokens"] = tokens;
    return user;
}

IUser::UserData User::userData() const
{
    UserData user;
    user.name = _userName;
    user.email = _eMail;
    user.group = _group;
    user.userID = _userID;
    user.lastActivity = lastActivity();
    user.userPermissions = _userPermissions;
    user.userData = _userData;
    return user;
}

QString User::group() const
{
    return _group;
}

void User::setGroup(const QString &group)
{
    _group = group;
}

QString User::getUserName() const
{
    return _userName;
}

void User::setUserName(const QString &userName)
{
    _userName = userName;
     Q_EMIT dataChanged();
}

QString User::getEMail() const
{
    return _eMail;
}

bool User::setEMail(const QString &eMail)
{
    _eMail = eMail;
     Q_EMIT dataChanged();
    return true;
}

bool User::isAuthorizedTo(QString permission)
{
    if(_userPermissions.value(IS_ADMIN).toBool())
    {
        return permission != SERVICE;
    }

    return _userPermissions.value(permission, false).toBool();
}


QString User::passHash() const
{
    return _passHash;
}

void User::setPassHash(const QString &passHash)
{
    _passHash = passHash;
}

QString User::identityID() const
{
    return _userID;
}

QString User::userLogin() const
{
    return _userID;
}

void User::setUserID(const QString &userID)
{
    _userID = userID;
     Q_EMIT dataChanged();
}

void User::addSteadyToken(QString token)
{
    _steadyTokens.insert(token);
}

void User::removeSteadyToken(QString token)
{
    _steadyTokens.remove(token);
}

QSet<QString> User::getSteadyTokens() const
{
    return _steadyTokens;
}

QVariantMap User::userPermissions() const
{
    return _userPermissions;
}

void User::setUserPermission(QString permission, bool allowed, bool last)
{
    _userPermissions.insert(permission, allowed);

    if(last)
    {
        Q_EMIT dataChanged();
        Q_EMIT userPermissionsChanged();
    }
}

QVariantMap User::getUserData() const
{
    return _userData;
}

bool User::checkPassword(QString password)
{
    return generateHash(password) == _passHash;
}

bool User::setPassword(QString password)
{
    setPassHash(generateHash(password));
    return true;
}

void User::setUserData(const QVariantMap &userData)
{
    _userData = userData;
    Q_EMIT dataChanged();
}


