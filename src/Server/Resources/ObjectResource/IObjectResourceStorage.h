/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IOBJECTRESOURCESTORAGE_H
#define IOBJECTRESOURCESTORAGE_H

#include <QObject>

class IObjectResourceStorage : public QObject
{
    Q_OBJECT

public:
    explicit IObjectResourceStorage(QObject *parent = nullptr) : QObject(parent = nullptr){}
    virtual bool        insertProperty(QString, QVariant) = 0;
    virtual bool        sync() = 0; // to write unsaved
    virtual bool        setMetadata(QVariant metadata) = 0;

    /*------   getter */

    virtual QVariant    getProperty(QString name) const = 0;
    virtual QVariantMap getAllProperties() const = 0;
    virtual QVariant    getMetadata() const  = 0;
};

#endif // IOBJECTRESOURCESTORAGE_H
