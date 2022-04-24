/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef SOCKETOBJECTHANDLER_H
#define SOCKETOBJECTHANDLER_H

#include <QObject>
#include "SocketCore/IResourceHandler.h"
#include "Connection/VirtualConnection.h"
#include "Server/Resources/ObjectResource/ObjectResource.h"

class SynchronizedObjectHandler : public IResourceHandler
{
    Q_OBJECT

public:
    explicit SynchronizedObjectHandler(QSharedPointer<ObjectResource> resource = QSharedPointer<ObjectResource> (nullptr));
    ~SynchronizedObjectHandler() override;
    void initHandle(ISocket* handle) override;
    bool dynamicContent() const override;
    bool isPermitted(QString token) const override;

private:
    QSharedPointer<ObjectResource> _resource;
    QList<ISocket*> _handles;

signals:
public slots:

private slots:
    void propertyChanged(QString property, QVariant data, iIdentityPtr user);
    void handleMessage(QVariant message, ISocket* handle) override;
};

#endif // SOCKETOBJECTHANDLER_H
