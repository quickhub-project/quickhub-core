/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef USER_H
#define USER_H

#include <QObject>
#include <QSet>
#include <QVariant>
#include "AuthentificationService.h"

#include "qhcore_global.h"
#include "IUser.h"

class COREPLUGINSHARED_EXPORT User : public IUser
{
    Q_OBJECT    
    friend class DefaultAuthenticator;

public:
    ~User() override;
    QString group() const;
    QString getUserName() const;
    QString getEMail() const override;
    QString passHash() const;
    QString identityID() const override;
    QString userLogin() const override;
    QVariantMap userPermissions() const;
    QVariantMap getUserData() const;
    void setUserName(const QString &getUserName) ;
    QSet<QString> getSteadyTokens() const override;

    bool checkPassword(QString password) override;
    void setUserData(const QVariantMap &userData);
    bool setEMail(const QString &getEMail) override;
    bool isAuthorizedTo(QString permission) override;
    QVariantMap toVariant() const ;
    UserData userData() const override;
    void setUserPermission(QString permision, bool allowed, bool last = true);

protected:
    explicit User(QObject *parent = nullptr);
    bool setPassword(QString password) override;
    explicit User(QVariantMap variant, QObject* parent = nullptr);
    void setUserID(const QString &identityID);
    void setPassHash(const QString &passHash);

    void setGroup(const QString &group);


    void addSteadyToken(QString token);
    void removeSteadyToken(QString token);

private:
    QString         _group;
    QString         _userName;
    QString         _eMail;
    QString         _passHash;
    QString         _userID;
    QVariantMap     _userData;
    QVariantMap     _userPermissions;
    QSet<QString>   _steadyTokens;

signals:
    void userNameChanged();
    void userPermissionsChanged();

public slots:


};

#endif // USER_H
