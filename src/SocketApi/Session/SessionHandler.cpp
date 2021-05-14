/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "SessionHandler.h"
#include <QJsonDocument>
#include "Server/Authentication/User.h"


SessionHandler::SessionHandler(QObject *parent) : IRequestHandler(parent)
{
    _authenticationService = AuthenticationService::instance();
    connect(_authenticationService, &AuthenticationService::sessionClosed, this, &SessionHandler::sessionClosed);
}

bool SessionHandler::handleRequest(QVariantMap message, ISocket *handle)
{
    QString command     = message["command"].toString();
    QString token       = message["token"].toString();
    QVariantMap payload = message["payload"].toMap();

    if(!command.startsWith("user"))
        return false;

    iUserPtr user = _authenticationService->getUserForToken(token);

    if(command == "user:login")
    {
        QString userId = payload["userID"].toString();
        QString password = payload["password"].toString();
        AuthenticationService::ErrorCode error;
        QString token = _authenticationService->login(userId, password, &error);
        qint64 tokenExpiration = _authenticationService->getTokenExpiration(token);
        QVariantMap answer;

        if(error == AuthenticationService::NoError)
        {
            connect(handle, SIGNAL(destroyed(QObject*)),this,SLOT(sessionConnectionDeleted(QObject*)));
            _tokenToHandleMap.insert(token, handle);
            handle->setProperty("token", token);
            answer["command"] = "user:login:success";
            QVariantMap payload;
            payload["token"] = token;
            payload["tokenExpiration"] = tokenExpiration;
            payload["user"] =  AuthenticationService::instance()->getUserForToken(token)->userData().toMap();
            answer["payload"] = payload;
            handle->sendVariant(answer);
            return true;
        }

        answer["errrorcode"] = error;
        answer["command"] = "user:login:failed";

        if(error == AuthenticationService::IncorrectPassword)
        {
            answer["errorstring"] = "Wrong password.";
        }

        if(error == AuthenticationService::UserNotExists)
        {
            answer["errorstring"] = "Unknown User.";
        }

        handle->sendVariant(answer);
        return true;
    }


    if(command == "user:add")
    {
        QVariantMap answer;
        QString password = payload["password"].toString();
        QString userId = payload["userID"].toString();
        QString eMail = payload["eMail"].toString();
        QString name = payload["name"].toString();

        AuthenticationService::ErrorCode err;
        iUserPtr user = DefaultAuthenticator::instance()->addUser(userId, password, token, &err);

        QSharedPointer<User> userObjPtr = qSharedPointerCast<User>(user);
        if(!userObjPtr.isNull())
        {
            user->setEMail(eMail);
            userObjPtr->setUserName(name);
        }

        if( err  == AuthenticationService::NoError)
        {
            answer["command"] = "user:add:success";
            qInfo()<<"User "+user->identityID()+" added successfully.";
        }
        else
        {
            answer["command"] = "user:add:failed";
            answer["errrorcode"] = err;
            if(err == AuthenticationService::InvalidData)
            {
                answer["errorstring"] = "Invalid or incomplete user data.";
            }

            if(err == AuthenticationService::UserAlreadyExists)
            {
                answer["errorstring"] = "User already exists.";
            }

            if( err == AuthenticationService::PermissionDenied)
            {
                answer["errorstring"] = "No permission to add user.";
            }
        }

        handle->sendVariant(answer);
        return true;
    }


    if(user.isNull())
    {
        QVariantMap answer;
        answer["command"] = command+":failed";
        answer["errorstring"] = "Token invalid. Please log in and try again.";
        handle->sendVariant(answer);
        return true;
    }

    if(command == "user:changepassword")
    {
        QVariantMap answer;
        QString userID = payload["userID"].toString();
        QString oldPassword = payload["oldPassword"].toString();
        QString newPassword = payload["newPassword"].toString();
        AuthenticationService::ErrorCode err = DefaultAuthenticator::instance()->changePassword(token, oldPassword, newPassword, userID);
        answer["errrorcode"] = err;
        if(err == AuthenticationService::NoError)
        {
            answer["command"] = "user:changepassword:success";
        }
        else
        {
            answer["command"] = "user:changepassword:failed";
            if(err == AuthenticationService::InvalidData)
                answer["errorString"] = "New password is empty.";

            if(err == AuthenticationService::IncorrectPassword)
                answer["errorString"] = "Password invalid.";

            if(err == AuthenticationService::UserNotExists)
                answer["errorString"] = "User not exists.";
        }
        handle->sendVariant(answer);
        return true;
    }

    if(command == "user:logout")
    {
        QVariantMap answer;
        if(!AuthenticationService::instance()->logout(token))
        {
            answer["command"] = "logout:failed";
            handle->sendVariant(answer);
        }
        return true;
    }

    if(command == "user:setpermission")
    {
        QVariantMap answer;
        QString permission = payload["permission"].toString();
        QString userID = payload["userID"].toString();    
        bool allowed = payload["allowed"].toBool();
        AuthenticationService::ErrorCode err = DefaultAuthenticator::instance()->setPermission(token, userID, permission, allowed);

        if(err == AuthenticationService::NoError)
        {
            answer["command"] = "user:setpermission:success";
        }
        else
        {
            answer["command"] = "user:setpermission:failed";
            answer["errrorcode"] = err;
        }

        handle->sendVariant(answer);
        return true;
    }


    if(command == "user:delete")
    {
        QString userID = payload["userID"].toString();
        QString password = payload["password"].toString();

        QVariantMap answer;
        AuthenticationService::ErrorCode error = DefaultAuthenticator::instance()->deleteUser(token, password, userID);
        answer["errrorcode"] = error;
        if(error == AuthenticationService::NoError)
        {
            answer["command"] = "user:delete:success";
        }
        else
        {
            answer["command"] = "user:delete:failed";

            if(error == AuthenticationService::PermissionDenied)
                answer["errorString"] = "Permission denied.";

            if(error == AuthenticationService::IncorrectPassword)
                answer["errorString"] = "Password invalid.";

            if(error == AuthenticationService::UserNotExists)
                answer["errorString"] = "User not exists.";
        }
        handle->sendVariant(answer);
        return true;
    }
    return false;
}

QStringList SessionHandler::getSupportedCommands()
{
    return QStringList();
}

void SessionHandler::init(QString storageDirectory)
{
    _dataStoragePath = storageDirectory;
}

void SessionHandler::sessionConnectionDeleted(QObject* obj)
{
    QVariant tokenVariant = obj->property("token");
    if(tokenVariant.isValid())
    {
        QString token = tokenVariant.toString();
        _tokenToHandleMap.remove(token);
        AuthenticationService::instance()->logout(token);
    }
}

void SessionHandler::sessionClosed(QString userID, QString token)
{
    Q_UNUSED (userID)
    ISocket* handle = _tokenToHandleMap.value(token, nullptr);
    if(handle)
    {
        QVariantMap msg;
        msg["command"] = "logout:success";
        handle->sendVariant(msg);
    }
}
