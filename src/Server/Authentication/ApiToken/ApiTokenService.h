/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef APITOKENSERVICE_H
#define APITOKENSERVICE_H

#include <QObject>
#include "../../Services/IService.h"

class ApiTokenService : public IService
{
    Q_OBJECT

public:
    explicit ApiTokenService(QObject* parent = nullptr);

    QString getServiceName() const override;
    QStringList getServiceCalls() const override;
    bool call(QString call, QString token, QString cbID, QVariant argument = QVariant()) override;
};

#endif // APITOKENSERVICE_H
