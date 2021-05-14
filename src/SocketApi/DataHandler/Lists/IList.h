/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef ILIST_H
#define ILIST_H

#include <QObject>
#include "QVariant"
#include "Connection/VirtualConnection.h"

class IList : public QObject
{
    Q_OBJECT

public:
    explicit IList(QObject *parent = nullptr);
    virtual QVariantList getListData() const = 0;
    virtual bool isPermitted(QString token) const;
    virtual void handleMessage(QVariant msg, ISocket *handle);

signals:
    void propertyChanged(QString property, QVariant data, int index);
    void itemChanged(QVariant item, int index);
    void itemAdded( QVariant item, int index);
    void itemRemoved(int index);

public slots:
};

#endif // ILIST_H
