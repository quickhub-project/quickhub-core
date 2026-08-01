/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef LISTACCESSPROXY_H
#define LISTACCESSPROXY_H

#include <QObject>
#include <QSharedPointer>
#include "ListResource.h"

class ListAccessProxy : public QObject
{
    Q_OBJECT

public:
    explicit ListAccessProxy(QSharedPointer<ListResource> resource, QObject* parent = nullptr);

    // --- Read-API (filtered when property filter is set) ---
    QVariantList getListData(iIdentityPtr identity) const;
    QVariant getItem(int idx, iIdentityPtr identity, QString uuid = "") const;
    QVariantMap getMetadata() const;
    int getCount() const;

    // --- Write-API (with property-level guard) ---
    IResource::ModificationResult appendItem(QVariant data, QString token);
    IResource::ModificationResult insertAt(QVariant data, int index, QString token);
    IResource::ModificationResult appendList(QVariantList data, QString token);
    IResource::ModificationResult setProperty(QString property, QVariant data, int index, QString uuid, QString token);
    IResource::ModificationResult set(QVariant data, int index, QString uuid, QString token);
    IResource::ModificationResult removeItem(QString uuid, QString token, int index = -1);
    IResource::ModificationResult deleteList(QString token);
    IResource::ModificationResult clearList(QString token);
    IResource::ModificationResult setMetadata(QVariant metadata);
    bool setFilter(QVariantMap query);

    // --- Utility ---
    QVariantMap filterItemForRead(const QVariantMap& item, iIdentityPtr identity) const;
    bool canReadProperty(const QString& property, iIdentityPtr identity) const;
    bool canWriteProperty(const QString& property, iIdentityPtr identity) const;

    // --- Resource access ---
    ListResource* resource() const;
    QSharedPointer<ListResource> resourcePtr() const;
    bool isPermittedToRead(iIdentityPtr identity) const;
    bool isPermittedToWrite(iIdentityPtr identity) const;
    bool dynamicContent() const;

private:
    QSharedPointer<ListResource> _resource;
};

#endif // LISTACCESSPROXY_H
