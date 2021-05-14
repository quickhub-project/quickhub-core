/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IMAGECOLLECTIONHANDLERFACTORY_H
#define IMAGECOLLECTIONHANDLERFACTORY_H


#include <QObject>
#include "../../SocketCore/IResourceHandlerFactory.h"

class ImageCollectionHandlerFactory : public IResourceHandlerFactory
{
    Q_OBJECT

public:
    explicit ImageCollectionHandlerFactory(QObject *parent = nullptr);
    virtual QString getResourceID(QString descriptor, QString token) const;
    virtual QString resourceTypeIdentifier() const;
    virtual IResourceHandler* createInstance(QString descriptor, QString token, Err::CloudError* error = 0);

signals:

public slots:
};

#endif // IMAGECOLLECTIONHANDLERFACTORY_H
