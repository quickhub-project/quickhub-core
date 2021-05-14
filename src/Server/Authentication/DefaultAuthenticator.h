/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef DEFAULTAUTHENTICATOR_H
#define DEFAULTAUTHENTICATOR_H

#include <QObject>
#include "IAuthenticator.h"
#include "User.h"
#include "AuthentificationService.h"


typedef QSharedPointer<User> userPtr;
class FileSystemLoader;
class DefaultAuthenticator : public IAuthenticator
{

    Q_OBJECT

public:    
    void init(QString userDataPath);

    iUserPtr getUser(QString userID) ;

    DefaultAuthenticator(QObject* parent = nullptr);

    /*!
        \fn void AuthenticationService::getUserForID(QString userID)
        Returns a userPtr for the given userID. If no user exists, the function will return a nullpointer.
        \note Don't forget to check if the returned pointer is valid!
    */
    userPtr getUserForID(QString userID);

    /*!
        \fn void AuthenticationService::addUser(QString userID, QString password, QString token, AuthenticationService::ErrorCode* error = 0)
        Call this function to add a new user with the ID \c username and the password \c password.
        The user which belongs to the session token needs have the appropriate permission for adding users.
    */
    userPtr addUser(QString userID, QString password, QString token, AuthenticationService::ErrorCode* error = nullptr);

    /*!
        \fn ErrorCode deleteUser(QString token, QString password = "", QString userID = "")
        Call this function to delete users.
        You can either use this function to delete your own user (You don't need to provide a userID in this case but the correct password for your account) or to delete other users.
        In the second case, you don't need to provide a password when the session user has the permissions to delete users.

    */
    AuthenticationService::ErrorCode deleteUser(QString token, QString password = "", QString userID = "");

    /*!
      \fn ErrorCode changePassword(QString token, QString oldPassword, QString newPassword, QString userID = "");
       If you want to change your own password, \c userID can be empty but the given password needs to be correct.
       If you want to change the password of another user, the session owner needs the apropriate permissions. \c oldPassword can be empty in this case.
    */
    AuthenticationService::ErrorCode changePassword(QString token, QString oldPassword, QString newPassword, QString userID = "");

    /*!
        \fn void AuthenticationService::getUsers(QString token ="", AuthenticationService::ErrorCode* error = 0)
        This function returns a list with all users. If you provide a token, the function will check the appropriate right level for you.
        \note It's highly recommended to use this functio with a token. If not, you need to care about the right level!
    */
    QVector<userPtr> getUsers(QString token ="", AuthenticationService::ErrorCode* error = nullptr);


    AuthenticationService::ErrorCode setPermission(QString token, QString userID, QString permission, bool allowed);


    const QVariantMap getData();

    static DefaultAuthenticator* instance();

    userPtr createUser(QString userID, QString password,  AuthenticationService::ErrorCode* error = nullptr);
    userPtr addUser(userPtr user, QString token, AuthenticationService::ErrorCode *error);

private:
    void deleteUser(userPtr user);
    void addUser(userPtr user);
    bool _everybodyCanAddUsers = false;
    QReadWriteLock _timestampLock;
    QString _userDataPath;
    QReadWriteLock _lock;
    QHash<QString, userPtr> _idToUserMap;
    QVector<userPtr> _users;
    QTimer _saveTimer;
    FileSystemLoader* _loader = nullptr;

private slots:
    void save();


signals:
    void userAdded(userPtr userID);
    void userDeleted(userPtr userID);

};

#endif // DEFAULTAUTHENTICATOR_H
