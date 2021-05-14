/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef IREQUESTHANDLER_H
#define IREQUESTHANDLER_H

#include <QObject>
#include "Connection/VirtualConnection.h"


class IRequestHandler : public QObject
{

public:
    explicit IRequestHandler(QObject* parent = nullptr):QObject(parent){}
    virtual ~IRequestHandler(){}
    virtual bool            handleRequest(QVariantMap message, ISocket* socket) = 0;
    virtual QStringList     getSupportedCommands() = 0;
    virtual void            init(QString storageDirectory){Q_UNUSED(storageDirectory)}

};

#endif // IREQUESTHANDLER_H
