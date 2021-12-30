/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


/*!
    \class Connection
    \brief This class capsules a QWebSocket connection.
    \ingroup connection handling

    If a client connects to the server, SocketServer will instanciate a new Connection object with the appropriate QWebSocket instance.
    Each instance of this class can have multiple sub-connections (VirtualConnection) which are derived from ISocket. ISockets can be attached
    to a resource or a service.

    That means one Websocket client application has only a single connection to the server, but can have multiple VirtualConnection objects which are communicating with the
    attached services.

    \note A Connection can't talk directly with servives. Only VirtualConnections can be attached to distinct services.

    \sa VirtualConnection AbstractSocket
*/


#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include "IConnectable.h"
#include <QThread>

class ISocket;
class VirtualConnection;
class Connection : public IConnectable
{
    Q_OBJECT

public:
    explicit    Connection(QWebSocket* socket, QObject *parent = nullptr);
    explicit    Connection(ISocket *socket, QObject *parent = nullptr);

    explicit    Connection(QObject *parent = nullptr);
                ~Connection() override;

    /*!
        Try to establish a connection to the given endpoint (e.g. server url)
    */
    void        connect(QString ident);

    /*!
        Send a QVariant to the endpoint. In most cases, the QVariant object will be serialized to JSON.
    */
    void        sendVariant(const QVariant &data) override;

    /*!
        Adds a virtual connection. Incoming messages for the registered VirtualConnections will be delivered after calling this function.
    */
    void        addVirtualConnection(VirtualConnection* connection);

    /*!
        Enables a keep-alive ping. Interval is the delay between two pings and timeout is the time after
        which the connection is terminated with a timeout error.
    */
    void        setKeepAlive(int interval, int timeout = 1000);

    /*!
        Returns true when the connection is connected to an endpoint.
    */
    bool        isConnected();

    /*!
        Returns the peer adress of the QWebSocket instance.
    */
    QString     getRemoteID() override;

    /*!
        Returns a pointer to the internal QWebSocket instance.
    */
    QWebSocket* getSocket();

private:
    void                                setupConnections();
    QWebSocket*                         _socket = nullptr;
    ISocket*                            _isocket;
    bool                                _connected;
    QHash<QString, VirtualConnection*>  _handles;
    bool                                _keepAlive = false;
    int                                 _pingInterval;
    int                                 _timeout;
    QTimer*                             _keepAliveTimer = nullptr;
    QTimer*                             _timeoutTimer =  nullptr;
    QThread*                            _trhead = nullptr;
    bool                                _binary = true;

signals:
    void connected();
    void disconnected();
    void socketError(QAbstractSocket::SocketError error);
    void doSendVariant(const QVariant& variant);

private slots:
    void socketDisconnected();
    void socketConnected();
    void variantMessageReceived(QVariant message);
    void binaryMessageReceived(QByteArray message);
    void textMessageReceived(QString message);
    void sendPing();
    void timeout();
    void handleDeleted();
    void invokeSendingVariant(const QVariant& data);


};

#endif // CONNECTIONHANDLER_H
