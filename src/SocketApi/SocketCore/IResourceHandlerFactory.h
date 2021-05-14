/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef IRESOURCEHANDLERFACTORY_H
#define IRESOURCEHANDLERFACTORY_H

#include <QObject>
#include "IResourceHandler.h"
#include "Server/Defines/ErrDef.h"

class IResourceHandlerFactory : public QObject
{
    Q_OBJECT

public:
    explicit IResourceHandlerFactory(QObject* parent = 0) : QObject(parent){};
    ~IResourceHandlerFactory(){};
    virtual IResourceHandler* createInstance(QString descriptor, QString token, Err::CloudError* error = 0) = 0;

    // resourceID will be used by Socket Resource Manager to manage handlers
    // two different descriptors can map to the same handler or vice versa. Only the UUID returned by this function
    // is a unique identifier to a distinct Resource.
    virtual QString getResourceID(QString descriptor, QString token) const = 0;
    virtual QString resourceTypeIdentifier() const = 0;
};



#endif // IRESOURCEHANDLERFACTORY_H

