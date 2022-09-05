/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IMAGERESOURCEFACTORY_H
#define IMAGERESOURCEFACTORY_H

#include <QObject>
#include "../ResourceManager/IResourceFactory.h"
#include "IImageResourceStorageFactory.h"

class ImageResourceFactory : public  IResourceFactory
{
    Q_OBJECT

public:
    ImageResourceFactory(IImageResourceStorageFactory *storageFactory, QObject* parent = 0);
    ImageResourceFactory(QObject* parent = 0);
    QString getResourceType() const override;
    void setAlternativeStorageFactory(IImageResourceStorageFactory *newAlternativeStorageFactory);

private:
    resourcePtr createResource(QString token, QString descriptor, QObject *parent) override;
    IImageResourceStorageFactory* _alternativeStorageFactory;
};

#endif // IMAGERESOURCEFACTORY_H
