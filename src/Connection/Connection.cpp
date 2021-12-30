/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "Connection.h"
#include "VirtualConnection.h"
#include <QDebug>
#include <QJsonDocument>
#include <QTimer>
#include <QtConcurrent>

void Connection::sendVariant(const QVariant& data)
{
    Q_EMIT doSendVariant(data);
}

Connection::Connection(QWebSocket *socket, QObject *parent): IConnectable(parent),
    _socket(socket),
    _connected(socket->state() > 0)
{
    socket->setParent(this);
    setupConnections();
}


Connection::Connection(ISocket* socket, QObject *parent):IConnectable(parent),
    _isocket(socket),
    _connected(socket->isConnected())
{
    socket->setParent(this);
    QObject::connect(socket, &ISocket::connected, this, &Connection::socketConnected);
    QObject::connect(socket, &ISocket::disconnected, this, &Connection::socketDisconnected);
    QObject::connect(socket, &ISocket::messageReceived, this, &Connection::variantMessageReceived);
    QObject::connect(this, &Connection::doSendVariant, this, &Connection::invokeSendingVariant, Qt::QueuedConnection);
}


Connection::Connection(QObject *parent) : IConnectable(parent),
    _socket(new QWebSocket()),
    _connected(false)
{
    setupConnections();
}

void Connection::setupConnections()
{
    QObject::connect(_socket, &QWebSocket::connected, this, &Connection::socketConnected);
    QObject::connect(_socket, &QWebSocket::disconnected, this, &Connection::socketDisconnected);
    QObject::connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(socketError(QAbstractSocket::SocketError)));
    QObject::connect(_socket,SIGNAL(binaryMessageReceived(QByteArray)), this,SLOT(binaryMessageReceived(QByteArray)));
    QObject::connect(_socket,SIGNAL(textMessageReceived(QString)), this,SLOT(textMessageReceived(QString)));
    QObject::connect(this, &Connection::doSendVariant, this, &Connection::invokeSendingVariant, Qt::QueuedConnection);
}


Connection::~Connection()
{
    if(_socket)
    {
        _socket->close(QWebSocketProtocol::CloseCodeGoingAway);
        qDebug()<<"Connection destroyed!";
        delete _socket;
        _socket = nullptr;
        qDebug()<<"Socket destroyed!";
    }
}

void Connection::connect(QString ident)
{
    if(_socket)
        _socket->open(ident);
}

void Connection::addVirtualConnection(VirtualConnection *connection)
{
    _handles.insert(connection->getUUID(), connection);
    QObject::connect(connection, &ISocket::destroyed, this, &Connection::handleDeleted);
}

void Connection::setKeepAlive(int interval, int timeout)
{
    _pingInterval = interval;
    _timeout = timeout;

    if(interval <= 0)
    {
        if (_keepAliveTimer)
        {
            _keepAliveTimer->stop();
            delete _keepAliveTimer;
            _keepAliveTimer = nullptr;
        }

        if(_timeoutTimer)
        {
            _timeoutTimer->stop();
            delete _timeoutTimer;
            _timeoutTimer = nullptr;
        }

        _keepAlive = false;
        return;
    }
    else
    {
        if (!_keepAliveTimer)
        {
            _keepAliveTimer = new QTimer(this);
            _keepAliveTimer->setSingleShot(true);
            QObject::connect(_keepAliveTimer, &QTimer::timeout, this, &Connection::sendPing);
        }

        if(!_timeoutTimer)
        {
            _timeoutTimer = new QTimer(this);
            _timeoutTimer->setSingleShot(true);
            QObject::connect(_timeoutTimer, &QTimer::timeout, this, &Connection::timeout);
        }

        _timeoutTimer->setInterval(_timeout);
        _keepAliveTimer->setInterval(_pingInterval);
        _keepAliveTimer->start();
        _pingInterval = interval;
        _timeout = timeout;
        _keepAlive = true;
    }
}

bool Connection::isConnected()
{
    return _connected;
}

QString Connection::getRemoteID()
{
    if(_socket)
        return _socket->peerAddress().toString();

    return "";
}

QWebSocket *Connection::getSocket()
{
    return _socket;
}

void Connection::socketDisconnected()
{
    _connected = false;
    Q_EMIT disconnected();
}

void Connection::socketConnected()
{
    _connected = true;
    Q_EMIT connected();
}

void Connection::handleDeleted()
{
    QObject* obj    = sender();
    QString key = _handles.key(static_cast<VirtualConnection*>(obj),"");
    if(key != "")
    {
        _handles.remove(key);
    }
}

void Connection::invokeSendingVariant(const QVariant &data)
{
    if(_socket)
    {
        if(_binary)
            _socket->sendBinaryMessage(QJsonDocument::fromVariant(data.toMap()).toJson());
        else
            _socket->sendTextMessage(QJsonDocument::fromVariant(data.toMap()).toJson());
    }
    else if(_isocket)
        _isocket->sendVariant(data);
}
void Connection::binaryMessageReceived(QByteArray message)
{
    _binary = true;
    QJsonParseError error;
    QVariantMap msg = QJsonDocument::fromJson(message, &error).toVariant().toMap();
    if(error.error != QJsonParseError::NoError)
    {
        qDebug()<<"Connection: Inavlid Json.";
        return;
    }
    variantMessageReceived(msg);
}

void Connection::variantMessageReceived(QVariant message)
{  
    QVariantMap msg = message.toMap();
    QString command = msg["command"].toString();

   if(_keepAlive)
   {
       _timeoutTimer->stop();
       _keepAliveTimer->start();
       if(msg["command"] == "pong")
       {
           return;
       }
   }

   QString uuid = msg["uuid"].toString();

   if(_handles.contains(uuid))
   {
       _handles.value(uuid)->deployMessage(msg);
    //   QtConcurrent::run(_handles.value(uuid), &VirtualConnection::deployMessage, msg );
       return;
   }

   if(command=="connection:register")
   {
       VirtualConnection* vconnection = new VirtualConnection(uuid, this);
       vconnection->deployMessage(msg);
       Q_EMIT newConnection(vconnection);
       return;
   }

   if( msg.isEmpty() || command == "ping")
   {
        QVariantMap pong;
        pong["command"] = "pong";
        sendVariant(pong);
        return;
   }

   if(uuid.isEmpty())
   {
       auto tmpHandles = _handles.values();
       QListIterator<VirtualConnection*> it(tmpHandles);
       while(it.hasNext())
       {
            qDebug()<<"No valid UUID -> Broadcast!";
            it.next()->deployMessage(msg);
       }
       return;
   }
}

void Connection::textMessageReceived(QString message)
{
    _binary = false;
    QJsonParseError error;
    QVariantMap msg = QJsonDocument::fromJson(message.toUtf8(), &error).toVariant().toMap();
    if(error.error != QJsonParseError::NoError)
    {
        qDebug()<<"Connection: Inavlid Json.";
        return;
    }
    variantMessageReceived(msg);
}

void Connection::sendPing()
{
    QVariantMap ping;
    ping["command"] = "ping";
    sendVariant(ping);
    _timeoutTimer->start();
}

void Connection::timeout()
{
    qDebug()<<"TIMEOUT";
    _connected = false;
    Q_EMIT disconnected();
}
