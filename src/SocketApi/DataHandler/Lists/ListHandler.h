/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef LISTHANDLER_H
#define LISTHANDLER_H

#include <QObject>
#include "../../SocketCore/IResourceHandler.h"

class IList;
class ListHandler : public IResourceHandler
{
    Q_OBJECT

public:
    explicit ListHandler(IList* list, QObject *parent = 0);

private:
    virtual void handleMessage(QVariant message, ISocket* handle) override;
    void initHandle(ISocket* handle) override;
    bool isPermitted(QString token) const override;

private:
    IList* _list = nullptr;

private slots:
    void propertyChanged(QString property, QVariant data, int index);
    void itemChanged(QVariant itemn, int index );
    void itemAdded(QVariant item, int index = -1);
    void itemRemoved(int index);
};

#endif // LISTHANDLER_H
