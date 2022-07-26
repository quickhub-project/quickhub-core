/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef SESSIONHANDLER_H
#define SESSIONHANDLER_H

#include <QObject>
#include <QWebSocket>
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/DefaultAuthenticator.h"
#include "SocketApi/SocketCore/IRequestHandler.h"

class SessionHandler : public IRequestHandler
{
    Q_OBJECT
public:
    explicit SessionHandler(QObject *parent = 0);

    bool            handleRequest(QVariantMap message, ISocket* handle);
    QStringList     getSupportedCommands();
    void            init(QString storageDirectory);

private:
    QString                                             _dataStoragePath;
    AuthenticationService*                              _authenticationService;
    QMap<QString, ISocket*>                   _tokenToHandleMap;


private slots:
    void sessionConnectionDeleted(QObject *obj);
    void sessionClosed(QString userID, QString token);

public slots:
};

#endif // SESSIONHANDLER_H
