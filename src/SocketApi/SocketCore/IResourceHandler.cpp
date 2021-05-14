/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "IResourceHandler.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/User.h"
#include <QUuid>
int IResourceHandler::instanceCount = 0;
IResourceHandler::IResourceHandler(QString resourceType, QObject *parent) : QObject(parent)
{

    _resourceType = resourceType;
    _timeoutTimer.setInterval(5000);
    connect(&_timeoutTimer, &QTimer::timeout, this, &IResourceHandler::checkTimeouts);
    _timeoutTimer.start();
}

bool IResourceHandler::attachHandle(QString token, ISocket *handle)
{
    if(!isPermitted(token))
        return false;

    handle->setParent(this);
    connect(handle, &ISocket::messageReceived, this, &IResourceHandler::messageReceived);
    connect(handle, &ISocket::disconnected,    this, &IResourceHandler::handleDisconnected);

    QString uid = QUuid::createUuid().toString();
    handle->setProperty("uid", uid);
    _handleMap.insert(uid, handle);
    _handles.insert(handle);

    QVariantMap msg;
    msg["command"] = _resourceType+":attach:success";
    handle->sendVariant(msg);
    initHandle(handle);
    return true;
}

QString IResourceHandler::getUUID()
{
    return "";
}

void IResourceHandler::initHandle(ISocket *handle)
{
    Q_UNUSED (handle)
}

bool IResourceHandler::isPermitted(QString token) const
{
    iUserPtr user = AuthenticationService::instance()->getUserForToken(token);
    return !user.isNull();
}

void IResourceHandler::detachHandle(ISocket *handle)
{
    if(_handles.contains(handle))
    {
        _handles.remove(handle);
        _handleMap.remove(_handleMap.key(handle));
        handle->setParent(nullptr);
        disconnect(handle, &ISocket::messageReceived, this, &IResourceHandler::messageReceived);
        disconnect(handle, &ISocket::disconnected,    this, &IResourceHandler::handleDisconnected);

        if(_handles.size() == 0)
        {
            //destroyed signal is catched in the handler. Object will be removed from the handler map.
            this->deleteLater();
        }
    }
}

bool IResourceHandler::dynamicContent() const
{
    return false;
}


void IResourceHandler::handleDisconnected()
{
    ISocket* handle = qobject_cast<ISocket *>(sender());
    if (handle)
    {
        detachHandle(handle);
    }
}

void IResourceHandler::messageReceived(QVariant message)
{
    ISocket* handle = qobject_cast<ISocket*>(sender());
    if(!handle)
        return;

    QVariantMap msgMap = message.toMap();

    QString command = msgMap["command"].toString();

    if(command == "ACK")
    {
        // see comment in line 174
       // QString uid = msgMap["msguid"].toString();
       // _sentMessages.remove(uid);
        return;
    }

    QStringList commandTokens = command.split(":");
    if(commandTokens.size() > 1 && commandTokens[1] == "detach")
    {
        QVariantMap answer;
        answer["command"] = _resourceType+":detach:success";
        handle->sendVariant(answer);
        detachHandle(handle);
        return;
    }
    handleMessage(message, handle);
}

void IResourceHandler::checkTimeouts()
{
    if(_sentMessages.count() == 0)
        return;

    QMutableMapIterator<QString, Message> it(_sentMessages);
    QStringList itemsToRemove;
    QSet<ISocket*> sockets;
    while(it.hasNext())
    {
        it.next();
        Message msg = it.value();
        QDateTime timestamp = msg.timestamp;
        QString receiverID  = msg.receiver;

        if(timestamp.addSecs(5) < QDateTime::currentDateTime())
        {
            QString peer;
            ISocket* receiver = _handleMap.value(receiverID, nullptr);
            if(receiver != nullptr)
            {
                IConnectable* connection = receiver->getConnection();
                if(connection != nullptr)
                    peer = connection->getRemoteID();

                sockets.insert(receiver);
            }

            qWarning()<< peer+" LOST MESSAGE --" << msg.data["command"].toString();
            itemsToRemove << it.key();
        }
    }

    QSetIterator<ISocket*> socketIt(sockets);
    while(socketIt.hasNext())
    {
        ISocket* socket = socketIt.next();
        Q_EMIT timeout(socket);
        initHandle(socket);
    }

    QListIterator<QString>removeIt(itemsToRemove);
    while(removeIt.hasNext())
        _sentMessages.remove(removeIt.next());
}


void IResourceHandler::deployToAll(QVariantMap msg, ISocket *sender, bool requestAck)
{
    Q_UNUSED(requestAck) //see commented out below
    QSetIterator<ISocket*> it(_handles);

    while(it.hasNext())
    {
        ISocket* receiver = it.next();
//        I guess there is a memory leak somewhere. The whole missing message detection Needs refactoring.
//        if(false)
//        {
//            QString msguid = QUuid::createUuid().toString();
//            msg["msguid"] = msguid;
//            Message cachedMessage;
//            cachedMessage.data       = msg;
//            cachedMessage.receiver   = receiver->property("uid").toString();
//            cachedMessage.timestamp  = QDateTime::currentDateTime();
//            _sentMessages.insert(msguid, cachedMessage);
//        }

        msg["reply"] = receiver == sender;
        receiver->sendVariant(msg);
    }
}
