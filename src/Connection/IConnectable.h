/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef ICONNECTABLE_H
#define ICONNECTABLE_H

#include <QObject>
#include "qhcore_global.h"

class ISocket;
class COREPLUGINSHARED_EXPORT IConnectable : public QObject
{
    Q_OBJECT

public:
    explicit IConnectable(QObject* parent = nullptr) : QObject(parent){}
    virtual void sendVariant(const QVariant &data) = 0;
    virtual QString getRemoteID(){return "";}
    ~IConnectable(){}

signals:
    void newConnection(ISocket* socket);
};


#endif // ICONNECTABLE_H
