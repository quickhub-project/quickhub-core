/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef APITOKENLISTRESOURCE_H
#define APITOKENLISTRESOURCE_H

#include <QObject>
#include "../../Resources/ListResource/ListResource.h"

class ApiTokenListResource : public ListResource
{
    Q_OBJECT

public:
    explicit ApiTokenListResource(QObject* parent = nullptr);

    QVariantList getListData() const override;
    int getCount() const override;
    ModificationResult appendItem(QVariant data, QString token) override;
    ModificationResult removeItem(QString uuid, QString token, int index = -1) override;
    bool isPermittedToRead(iIdentityPtr identity) const override;
    bool isPermittedToWrite(iIdentityPtr identity) const override;
    QVariant getItem(int idx, QString uuid = "") const;

private slots:
    void onTokenCreated(const QString& uuid);
    void onTokenDeleted(const QString& uuid);
};

#endif // APITOKENLISTRESOURCE_H
