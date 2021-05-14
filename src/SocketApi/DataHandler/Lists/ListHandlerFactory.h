/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef LISTHANDLERFACTORY_H
#define LISTHANDLERFACTORY_H

#include <QObject>
#include "../../SocketCore/IResourceHandlerFactory.h"

class ListHandlerFactory : public IResourceHandlerFactory
{
    Q_OBJECT
public:
    explicit ListHandlerFactory(QObject *parent = 0);

    virtual IResourceHandler* createInstance(QString descriptor, QString token, Err::CloudError* error);

    // resourceID will be used by Socket Resource Manager to manage handlers
    // two different descriptors can map to the same handler or vice versa. Onl the UUID returned by this function
    // is a unique identifier to a distinct Resource.
    virtual QString getResourceID(QString descriptor, QString token) const;
    virtual QString resourceTypeIdentifier() const;

signals:

public slots:
};

#endif // LISTHANDLERFACTORY_H
