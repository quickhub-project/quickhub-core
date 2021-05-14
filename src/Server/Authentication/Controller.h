/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "IIdentitiy.h"
#include <QObject>
#include <QMap>

class Controller : public QObject, public IIdentity
{
    Q_OBJECT

public:
    explicit Controller(QObject *parent = nullptr);

    /*!
        \fn virtual bool isAuthorizedTo(QString permission);
        Returns true if the identity has the apropriate permission.
    */
    virtual bool            isAuthorizedTo(QString permission);
    virtual QString         identityID() const;

private:
    QString             _identityID;
    QMap<QString, bool> _permissions;


signals:

};

#endif // CONTROLLER_H
