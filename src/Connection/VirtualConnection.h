/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef VIRTUALCONNECTION_H
#define VIRTUALCONNECTION_H

#include <QObject>
#include <QUuid>

#include "Connection.h"
#include "ISocket.h"

class VirtualConnection : public ISocket
{
    Q_OBJECT

    enum ConnectionState
    {
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
        DISCONNECTED
    };

public:
    explicit    VirtualConnection(Connection* connection = nullptr);
                ~VirtualConnection() override;
    explicit    VirtualConnection(QString uuid, Connection* connection = nullptr);
    QString     getUUID();
    Connection* getConnection() override;
    void        setKeepAlive(int interval, int timeout = 1000) override;

    //          make connection as friend and do private
    void        deployMessage(const QVariantMap &message);
    bool        isConnected() override;

public slots:
   void sendVariant(const QVariant &data) override;


private:
    ConnectionState     _state;
    Connection*         _connection;
    QString             _uuid;
    bool                _connected;

private slots:
    void close();
    void open();
    void connectionConnected();
    void connectionDisconnected();
    void connectionDestroyed();

};

#endif // VIRTUALCONNECTION_H
