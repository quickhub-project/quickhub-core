/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef OBJECTACCESSPROXY_H
#define OBJECTACCESSPROXY_H

#include <QObject>
#include <QSharedPointer>
#include "ObjectResource.h"

class ObjectAccessProxy : public QObject
{
    Q_OBJECT

public:
    explicit ObjectAccessProxy(QSharedPointer<ObjectResource> resource, QObject* parent = nullptr);

    // --- Read-API (filtered when property filter is set) ---
    QVariantMap getObjectData(iIdentityPtr identity) const;
    QVariantMap getMetaData() const;

    // --- Write-API (with property-level guard) ---
    IResource::ModificationResult setProperty(QString name, const QVariant& value, QString token);

    // --- Utility ---
    QVariantMap filterObjectForRead(const QVariantMap& objectData, iIdentityPtr identity) const;
    bool canReadProperty(const QString& property, iIdentityPtr identity) const;
    bool canWriteProperty(const QString& property, iIdentityPtr identity) const;

    // --- Resource access ---
    ObjectResource* resource() const;
    QSharedPointer<ObjectResource> resourcePtr() const;
    bool isPermittedToRead(iIdentityPtr identity) const;
    bool isPermittedToWrite(iIdentityPtr identity) const;
    bool dynamicContent() const;
    bool setFilter(QVariantMap query);

private:
    QSharedPointer<ObjectResource> _resource;
};

#endif // OBJECTACCESSPROXY_H
