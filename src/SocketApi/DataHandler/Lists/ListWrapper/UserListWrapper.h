/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef USERLISTWRAPPER_H
#define USERLISTWRAPPER_H

#include <QObject>
#include "../IList.h"
#include <QSignalMapper>
#include "Server/Authentication/User.h"
#include "Server/Authentication/DefaultAuthenticator.h"

class UserListWrapper : public IList
{
    Q_OBJECT

public:
    explicit UserListWrapper(QObject *parent = nullptr);
    virtual QVariantList getListData() const override;
    bool isPermitted(QString token) const override;

private:
    QSignalMapper               _mapper;
    QMap<QString, QVariant>     _userVariants;
    QVariantMap                 userToVariant(userPtr user) const;
    int getIndex(QString uuid) const;



public slots:
    void dataChanged(QString userID);
    void userAdded(userPtr user);
    void userDeleted(userPtr user);

signals:
};

#endif // USERLISTWRAPPER_H
