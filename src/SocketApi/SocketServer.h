/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>


#include "Connection/VirtualConnection.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Connection/Connection.h"
#include <QWebSocketCorsAuthenticator>

#define STORAGE_PATH                _serverRootPath+"data/"
#define CONFIG_DATA                 _serverRootPath+"config/"
#define USER_DATA                   _serverRootPath+"config/users"
#define DEVICE_DATA                 _serverRootPath+"devices"

#define SSL_CERTIFICATE             _serverRootPath+"config/certificates/server.crt"
#define SSL_KEY                     _serverRootPath+"config/certificates/server.key"

class IRequestHandler;
class ListResourceFactory;
class ImageResourceFactory;
class ObjectResourceFactory;
class IObjectResourceStorageFactory;
class IListResourceStorageFactory;
class IImageResourceStorageFactory;
class SocketServer : public QObject
{
    Q_OBJECT

public:
    void        addRequestHandler(IRequestHandler* handler);
    explicit    SocketServer(QObject *parent = nullptr);
    void        setObjectResourceStorageFactory(IObjectResourceStorageFactory* factory);
    void        setListResourceStorageFactory(IListResourceStorageFactory* factory);
    void        setImageResourceStorageFactory(IImageResourceStorageFactory* factory);
    static      SocketServer* instance();

private:
    void                                    initServices();
    bool                                    initSecureServer();
    void                                    initNonSecureServer();

    QWebSocketServer*                       _server = nullptr;
    QString                                 _serverRootPath;
    QString                                 _dataStoragePath;
    QString                                 _usersPath;
    QList<QWebSocket*>                      _allConnections;
    AuthenticationService*                  _authenticationService = nullptr;;
    QVector<IRequestHandler*>               _handlers;
    QVector<Connection*>                    _connections;
    ListResourceFactory*                    _listResourceFactory = nullptr;
    ObjectResourceFactory*                  _objectResourceFactory = nullptr;
    ImageResourceFactory*                   _imageResourceFactory = nullptr;
    QList<QThread*>                         _threadPool;
    int                                     _port;


public slots:
    void        newVirtualConnection(ISocket* handle);
    void        start(QString storageDirectory, quint16 port);

private slots:
    void newConnection();
    void messageReceived(QVariant message);
    void handleDisconnected();
//    void handleDestroyed();
    void connectionDisconnected();
    void sslError(const QList<QSslError> &errors);
    void serverError(QWebSocketProtocol::CloseCode closeCode);
    void acceptError(QAbstractSocket::SocketError socketError);
    void peerVerifyError(const QSslError &error);
};

#endif // SOCKETSERVER_H
