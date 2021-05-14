/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef SOCKETLISTHANDLERFACTORY_H
#define SOCKETLISTHANDLERFACTORY_H

#include <QObject>
#include "../../SocketCore/IResourceHandlerFactory.h"

class SynchronizedListHandlerFactory : public IResourceHandlerFactory
{
    Q_OBJECT

public:
    explicit SynchronizedListHandlerFactory(QObject* parent = nullptr);
    IResourceHandler* createInstance(QString path, QString token, Err::CloudError *error) override;
    QString resourceTypeIdentifier() const override;
    QString getResourceID(QString descriptor, QString token) const override;

};

#endif // SOCKETLISTHANDLERFACTORY_H
