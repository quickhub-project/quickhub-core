/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include <QJsonDocument>
#include <QSslKey>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QThread>

#include "SocketServer.h"
#include "Server/Authentication/User.h"
#include "Server/Resources/ResourceManager/ResourceManager.h"
#include "SocketCore/IRequestHandler.h"
#include "Session/SessionHandler.h"

#include "SocketCore/SocketResourceManager.h"
#include "ResourceHandler/SynchronizedList/SynchronizedListHandlerFactory.h"
#include "ResourceHandler/SynchronizedObject/SynchronizedObjectHandlerFactory.h"
#include "ResourceHandler/ImageCollection/ImageCollectionHandlerFactory.h"
#include "Server/Devices/DeviceManager.h"
#include "Server/Resources/ListResource/ListResourceFactory.h"
#include "Server/Resources/ObjectResource/ObjectResourceFactory.h"
#include "Server/Resources/ImageResource/ImageResourceFactory.h"
#include "Storage/FileSystemPaths.h"
#include "Devices/SocketDeviceHandler.h"
#include "DataHandler/Lists/ListHandlerFactory.h"
#include "Devices/DeviceHandleHandlerFactory.h"
#include "Services/ServiceRequestHandler.h"
#include "Server/Settings/SettingsManager.h"

Q_GLOBAL_STATIC(SocketServer, socketServer);

SocketServer::SocketServer(QObject *parent) : QObject(parent)
{

}

SocketServer *SocketServer::instance()
{
    return socketServer;
}


void SocketServer::addRequestHandler(IRequestHandler *handler)
{
    qInfo()<<"Added external RequestHandler";
    _handlers.append(handler);
}

void SocketServer::start(QString storageDirectory, quint16 port)
{
    _serverRootPath = storageDirectory;
    _port = port;

    qRegisterMetaType<QAbstractSocket::SocketState>();
    initServices();

    if(!initSecureServer())
       initNonSecureServer();
    if (_server->listen(QHostAddress::Any, _port))
    {
        qDebug()<<"Websocket-Server up and running. Listening on port"<< _port<<".";
        qDebug()<<"Data will be saved in"<< _serverRootPath;
        connect(_server, &QWebSocketServer::newConnection, this, &SocketServer::newConnection);
    }
    else
        qDebug()<< "Could not open WebSocket-Server."+ _server->errorString();
}

void SocketServer::initServices()
{
    FileSystemPaths::instance()->setStoragePath(STORAGE_PATH);
    FileSystemPaths::instance()->setConfigPath(CONFIG_DATA);

    DeviceManager::instance()->init(DEVICE_DATA);
    DefaultAuthenticator::instance()->init(USER_DATA);
    AuthenticationService::instance()->registerAuthenticator(DefaultAuthenticator::instance());

    _authenticationService = AuthenticationService::instance();
    _authenticationService->setParent(this);

    ResourceManager* resourceManager = ResourceManager::instance();
    resourceManager->addResourceFactory(new ListResourceFactory(this));
    resourceManager->addResourceFactory(new ObjectResourceFactory(this));
    resourceManager->addResourceFactory(new ImageResourceFactory(this));
    resourceManager->addResourceFactory(SettingsManager::instance());

    _handlers << new SessionHandler(this);
    _handlers << new SocketDeviceHandler(this);
    _handlers << new ServiceRequestHandler(this);

    SocketResourceManager* manager = new SocketResourceManager(this);
    manager->registerFactory(new SynchronizedListHandlerFactory(this));
    manager->registerFactory(new ImageCollectionHandlerFactory(this));
    manager->registerFactory(new SynchronizedObjectHandlerFactory(this));
    manager->registerFactory(new DeviceHandleHandlerFactory(this));
    manager->registerFactory(new ListHandlerFactory(this));
    _handlers.append(manager);
}

bool SocketServer::initSecureServer()
{
    QString certificatePath = QProcessEnvironment::systemEnvironment().value("SSL_CERT", SSL_CERTIFICATE);
    QString keyPath = QProcessEnvironment::systemEnvironment().value("SSL_KEY", SSL_KEY);

    if (!QFile::exists(certificatePath))
    {
        qDebug()<<"No SSL certificate found in "+ certificatePath;
        return false;
    }

    if (!QFile::exists(keyPath))
    {
        qDebug()<<"No SSL key found in "+ SSL_KEY;
        return false;
    }

    qDebug()<<"SSL certificate found in "+ certificatePath + ", key found in "+ keyPath;
    _server = new QWebSocketServer(QStringLiteral("QHomeAutomationServer"), QWebSocketServer::SecureMode, this);
    connect(_server, &QWebSocketServer::sslErrors, this, &SocketServer::sslError);
    connect(_server, &QWebSocketServer::serverError, this, &SocketServer::serverError);
    connect(_server, &QWebSocketServer::acceptError, this, &SocketServer::acceptError);

    QSslConfiguration sslConfiguration;
    QFile certFile(certificatePath);
    QFile keyFile(keyPath);
    certFile.open(QIODevice::ReadOnly);
    keyFile.open(QIODevice::ReadOnly);
    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    certFile.close();
    keyFile.close();
    sslConfiguration.setPeerVerifyMode(QSslSocket::AutoVerifyPeer);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setProtocol(QSsl::TlsV1_2);
    _server->setSslConfiguration(sslConfiguration);
    return true;
}

void SocketServer::initNonSecureServer()
{
    _server = new QWebSocketServer(QStringLiteral("QuickHub"), QWebSocketServer::NonSecureMode, this);
    qWarning()<<"ATTENTION -- SERVER WILL BE STARTED WITHOUT SSL ENCRYPTION !!!";
}

void SocketServer::newConnection()
{
    QWebSocket* newSocket = _server->nextPendingConnection();
    Connection* connection = new Connection(newSocket,  QCoreApplication::instance());
    qInfo() << connection->getRemoteID() <<" has connected.";
    _connections << connection;
    connect(connection, &Connection::newConnection, this, &SocketServer::newVirtualConnection);
    connect(connection, &Connection::disconnected, this, &SocketServer::connectionDisconnected);
}

void SocketServer::newVirtualConnection(ISocket *handle)
{
    handle->setParent(this);
    connect(handle, &ISocket::messageReceived, this, &SocketServer::messageReceived);
    connect(handle, &ISocket::disconnected, this, &SocketServer::handleDisconnected);
}


void SocketServer::messageReceived(QVariant message)
{
    ISocket* handle = qobject_cast<ISocket *>(sender());
    QVariantMap msg = message.toMap();

    if(handle->parent() == nullptr)
    {
        handle->setParent(this);
    }

    // Handle is owned by an service
    if(handle->parent() != this)
        return;

    QVectorIterator<IRequestHandler*> it(_handlers);
    while(it.hasNext())
    {
        IRequestHandler* handler = it.next();
        if (handler->handleRequest(message.toMap(), handle))
        {
            return;
        }
    }
}

void SocketServer::handleDisconnected()
{
    ISocket* handle = qobject_cast<ISocket *>(sender());
    if (handle)
    {
        handle->deleteLater();
    }
}

void SocketServer::connectionDisconnected()
{
    Connection* connection = qobject_cast<Connection*>(sender());
    if(connection)
    {
        qInfo()<< connection->getRemoteID() <<" has disconnected.";
        _connections.removeAll(connection);
        // virtual connections will catch the destroyed signal!
        connection->deleteLater();
    }
}

void SocketServer::sslError(const QList<QSslError>& errors)
{
    qDebug()<<Q_FUNC_INFO<<" "<<errors;
}

void SocketServer::serverError(QWebSocketProtocol::CloseCode closeCode)
{
    qDebug()<<Q_FUNC_INFO<<" "<<closeCode;
}

void SocketServer::acceptError(QAbstractSocket::SocketError socketError)
{
    qDebug()<<Q_FUNC_INFO<<" "<<socketError;
}

void SocketServer::peerVerifyError(const QSslError &error)
{
    Q_UNUSED (error)
    qDebug()<<Q_FUNC_INFO;
}
