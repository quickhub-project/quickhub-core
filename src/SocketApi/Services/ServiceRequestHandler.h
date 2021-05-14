/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef SERVICEREQUESTHANDLER_H
#define SERVICEREQUESTHANDLER_H

#include <QObject>
#include <QPointer>
#include "../SocketCore/IRequestHandler.h"

class ServiceRequestHandler : public IRequestHandler
{
    Q_OBJECT

public:
    explicit ServiceRequestHandler(QObject *parent = nullptr);
    virtual bool            handleRequest(QVariantMap message, ISocket* socket);
    virtual QStringList     getSupportedCommands();

signals:

public slots:
    void sendAnswer(QString uid, QVariant result);

private:
    QMap<QString, QPointer<ISocket>> _socketMap;
};

#endif // SERVICEREQUESTHANDLER_H
