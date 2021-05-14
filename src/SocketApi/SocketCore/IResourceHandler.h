/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


/*!
    \class IResourceHandler
    \brief A resource handler implements the server side WebSocket API  protocol for a resource. This is the abstract base class for all resource handlers.
    \ingroup WebSocket API

    Each client connected to a distinct resource is represented by an ISocket instance attached to the appropriate resource handler.
    If a client wants to modify a resource (e.g. add an item to a list), the virtual function handleMessage() will be called with the appropriate api command message.
    If the modification succeeded, the api command message will be forwarded to all the other clients which are also attached to the resource handler. Thus all clients are in sync again.

    The principle reminds several participants who are in a chatroom (where the chatroom is the ResourceHandler). If the message of one participant arrives at the server, it is forwarded to
    all other connected participants.
    \sa IResourceHandlerFactory, SocketResourceManager
*/

#ifndef ISOCKETRESOURCEHANDLER_H
#define ISOCKETRESOURCEHANDLER_H

#include <QObject>
#include <QPointer>
#include "Server/Resources/ResourceManager/IResource.h"
#include "Connection/VirtualConnection.h"

class ISocket;
class IResourceHandler : public QObject
{
    Q_OBJECT

public:

    struct Message
    {
        QDateTime timestamp;
        QString   receiver;
        QVariantMap data;

    };

    IResourceHandler(QString resourceType, QObject* parent = nullptr);
    virtual ~IResourceHandler(){/*qDebug()<<Q_FUNC_INFO<< --instanceCount;*/}


    /*!
        \fn void IResourceHandler::attachHandle(QString token, ISocket* handle);
        This function adds a new ISocket to to the resource handler. This function is called in SocketResourceManager when a new client
        connects to a resource.
        \note \c isPermitted() will be called internally to check if the corresponding user is permitted to connect to this resource.
    */
    virtual bool attachHandle(QString token, ISocket* handle);

    /*!
        \fn bool isPermitted(QString token) const;
        This function can be overwritten to decide wether a specific user has access to this resource or not. It will be called within \c attachHandle()
        By default, it returns true if the token belongs to a user which is logged in.
    */
    virtual bool isPermitted(QString token) const;


    /*!
        \fn void getResourcePtr();
        Returns a pointer to the resource.
    */
    virtual bool dynamicContent() const;

protected:
    QSet<ISocket*>          _handles;
    QMap<QString, QPointer<ISocket>> _handleMap;

    virtual QString getUUID();

    /*!
        \fn virtual void deployToAll(QVariantMap msg, ISocket* sender = 0);
        This function will send the given QVariantMap as json string via all attached ISocket handles.
        If you answer to a message, you can provide the pointer to the origin sender. The message to the sender will then have a special flag.
    */
    void deployToAll(QVariantMap msg, ISocket* sender = nullptr, bool requestAck = true);


    /*!
        \fn virtual void initHandle(ISocket* handle)
        This function is called after a new ISocket instance has been succesfully attached to the server.
        In most cases, the first init message from the server contains a full snapshot of the resource.
        After this, only deltas will sent to the clients to keep the data in sync.
    */
    virtual void initHandle(ISocket* handle);

    /*!
        \fn void detachHandle(ISocket* handle);
        \sa attachHandle(QString token, ISocket* socket)
    */
    void detachHandle(ISocket* handle);


private:
    QString                     _resourceType;
    QMap<QString, Message>      _sentMessages;
    QTimer                      _timeoutTimer;
    static int                  instanceCount;


signals:
    void timeout(ISocket* resource);

private slots:
     void handleDisconnected();
     void messageReceived(QVariant message);
     void checkTimeouts();

     /*!
         This is the function which will be called whenever a connected client sends a message.
         The message needs to be evaluated, the resource if need be modified and all other connected clients notified.
     */
     virtual void handleMessage(QVariant message, ISocket* handle) = 0;


};


#endif // ISOCKETRESOURCEHANDLER_H
