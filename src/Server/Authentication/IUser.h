/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IUSER_H
#define IUSER_H

#include <QObject>
#include <QSet>
#include <QVariant>
#include <QDebug>
#include <QProcessEnvironment>
#include "IIdentitiy.h"

/*!
    \class IUser
    \brief This class is the abstract interface for user objects
    Derive from this class together with an IAuthenticator instance to integrate your own user database.
    \sa IAuthenticator
*/

class IUser : public QObject, public IIdentity
{
    Q_OBJECT

public:

    struct UserData
    {
        QString name;
        QString userID;
        QString email;
        QString group;
        QVariantMap userPermissions;
        QVariantMap userData;
        qint64 lastActivity = -1;

        QVariantMap toMap()
        {
             QVariantMap user;
             user["group"] = group;
             user["eMail"] = email;
             user["userName"] = name;
             user["userID"] = userID;
             user["userPermissions"] = userPermissions;
             user["lastActivity"] = lastActivity;
             user["userData"]  = userData;
             return user;
        }
    };

    IUser(QObject* parent = nullptr) : QObject(parent){
        _sessionExpiration = QProcessEnvironment::systemEnvironment().value("USER_SESSION_EXPIRATION", "1200").toInt();
    }
    virtual ~IUser(){}

    /*!
        \fn void IUser::addToken(QString token)
        This function is exclusively user by AuthenticationService when a user logs in.
        A session token will be added to the user.
    */
    void   addToken(QString token) override;

    /*!
        \fn void IUser::removeToken(QString token)
        This function is exclusively user by AuthenticationService when a user logs out.
        The session token will be removed from its user.
    */
    void   removeToken(QString token) override;

    /*!
        \fn void IUser::removeAllTokens()
        This function removes all session tokens.
    */
    void   removeAllTokens();

    /*!
        \fn void IUser::removeAllTokens()
        Returns the number ov active sessions. So how often this user is logged in.
    */
    int    sessionCount();

    /*!
        \fn QSet<QString> IUser::getTokens() const
        Returns a set with all active session tokens.
    */
    QSet<QString>           getTokens() const;

    /*!
        \fn QSet<QString> IUser::getSteadyTokens() const
        Returns a set with all steady tokens.These are tokens that are persisted and permanently valid
        without the need for a new login. This can be useful in connection with REST APIs.
    */
    virtual QSet<QString>   getSteadyTokens() const;

    /*!
        \fn QSet<QString> IUser::checkPassword(QString password) const
        This function must be implemented if you want to work with your own user objects. Return true if
        the given password to this user is correct.
    */
    virtual bool            checkPassword(QString password) = 0;

    /*!
        \fn QSet<QString> IUser::setPassword(QString password) const
        Store the new password (no, not the password - use hashes!). Return false when something went wrong.
    */
    virtual bool            setPassword(QString password) = 0;

    /*!
        \fn QString IUser::userLogin() const = 0;
        Here the login string must be returned, this can be the eMail adress or a explicit login or nickname.
    */
    virtual QString         userLogin() const = 0;

    /*!
        \fn QString IUser::setEMail(const QString &eMail) = 0;
        This function is called when a user changes its eMail adress. Make sure to persist the desired eMail Adress.
    */
    virtual bool            setEMail(const QString &eMail) = 0;

    /*!
        \fn QString IUser::getEMail() const = 0;
        Return the new eMail Adress.
    */
    virtual QString         getEMail() const = 0;

    /*!
        \fn QString IUser::getEMail() const = 0;
        Return the whole blob of user related data.
    */
    virtual UserData        userData() const = 0;

    virtual void            setSessionExpiration(int timeout);
    virtual int             sessionExpiration() const override {return _sessionExpiration;};
    bool                    multipleSessionsAllowed() const override {return true;};

private:
    QSet<QString>   _tokens;
    int             _sessionExpiration;
    static int      instanceCount;

protected:
    QString generateHash(QString pass);

signals:
    void sessionCountChanged();
    void dataChanged();
};
#endif // IUSER_H
