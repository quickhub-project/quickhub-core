/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef ICONNECTIONCHANNEL_H
#define ICONNECTIONCHANNEL_H

#include <QObject>
#include "qhcore_global.h"

class IConnectable;
class COREPLUGINSHARED_EXPORT ISocket : public QObject
{
    Q_OBJECT

public:
    explicit ISocket(QObject *parent = nullptr):QObject(parent){}
    virtual ~ISocket(){}
    virtual void sendVariant(const QVariant &data) = 0;
    virtual void setKeepAlive(int interval, int timeout) = 0 ;
    virtual bool isConnected() = 0;
    virtual IConnectable* getConnection(){return nullptr;}

signals:
    void connected();
    void disconnected();
    void messageReceived(const QVariant& message);

public slots:
};

#endif // ICONNECTIONCHANNEL_H
