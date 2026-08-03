/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef SOCKETOBJECTHANDLER_H
#define SOCKETOBJECTHANDLER_H

#include <QObject>
#include <functional>
#include "SocketCore/IResourceHandler.h"
#include "Connection/VirtualConnection.h"
#include "Server/Resources/ObjectResource/ObjectResource.h"
#include "Server/Resources/ObjectResource/ObjectAccessProxy.h"

class SynchronizedObjectHandler : public IResourceHandler
{
    Q_OBJECT

public:
    struct PropertyChangeEvent
    {
        QString property;
        QVariant data;
        iIdentityPtr user;
    };
    explicit SynchronizedObjectHandler(QSharedPointer<ObjectResource> resource = QSharedPointer<ObjectResource> (nullptr));
    ~SynchronizedObjectHandler() override;
    void initHandle(ISocket* handle) override;
    bool dynamicContent() const override;
    bool isPermitted(QString token) const override;

private:
    QSharedPointer<ObjectResource> _resource;
    ObjectAccessProxy* _proxy;
    QList<ISocket*> _handles;
    void deployToAllFiltered(QVariantMap msg, std::function<QVariantMap(QVariantMap, iIdentityPtr)> filterFn);

signals:

public slots:

private slots:
    void propertyChanged(QString property, QVariant data, iIdentityPtr user);
    void handleMessage(QVariant message, ISocket* handle) override;
    void sendEvent(QVariantMap data);
};

#endif // SOCKETOBJECTHANDLER_H
