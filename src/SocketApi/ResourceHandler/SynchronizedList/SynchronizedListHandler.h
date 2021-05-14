/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef SYNCHRONIZEDLISTBACKEND_H
#define SYNCHRONIZEDLISTBACKEND_H

#include <QObject>
#include "Connection/VirtualConnection.h"
#include "Server/Resources/ListResource/ListResource.h"
#include <QVariant>

#include "../../SocketCore/IResourceHandler.h"


class SynchronizedListHandler : public IResourceHandler
{
    Q_OBJECT

public:
    explicit SynchronizedListHandler(QSharedPointer<ListResource> resource);
    ~SynchronizedListHandler() override;
    void initHandle(ISocket* handle) override;
    bool dynamicContent() const override;
    bool isPermitted(QString token) const override;

private:
    QSharedPointer<ListResource> _resource;
    // checks if item at index has correct uuid. if not, the correct index will be searched.
    int getIndexForUUID(QString UUID);
    void handleMessage(QVariant message, ISocket* handle) override;

private slots:
    void metadataChanged();
    void itemAppended(QVariant data, iIdentityPtr  user);
    void itemInserted(QVariant data, int index, iIdentityPtr  user);
    void listAppended(QVariantList data, iIdentityPtr  user);
    void itemRemoved(int index, QString uuid, iIdentityPtr  user);
    void listDeleted(iIdentityPtr  user);
    void listCleared(iIdentityPtr  user);
    void itemSet(QVariant data, int index, QString uuid, iIdentityPtr  user);
    void propertySet(QString property, QVariant data, int index, QString uuid, iIdentityPtr user, qint64 timestamp);
    void listResetted();
signals:

public slots:
};

#endif // SYNCHRONIZEDLISTBACKEND_H
