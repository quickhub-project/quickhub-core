/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef DEVICEHANDLEHANDLERFACTORY_H
#define DEVICEHANDLEHANDLERFACTORY_H

#include <QObject>
#include "../SocketCore/IResourceHandlerFactory.h"

class DeviceHandleHandlerFactory : public IResourceHandlerFactory
{
    Q_OBJECT

public:
    DeviceHandleHandlerFactory(QObject* parent = nullptr);
    IResourceHandler* createInstance(QString descriptor, QString token, Err::CloudError *error) override;
    QString resourceTypeIdentifier() const override;
    QString getResourceID(QString descriptor, QString token) const override;
};

#endif // DEVICEHANDLEHANDLERFACTORY_H
