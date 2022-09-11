/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef OBJECTRESOURCEFACTORY_H
#define OBJECTRESOURCEFACTORY_H

#include "../ResourceManager/IResourceFactory.h"
#include "IObjectResourceStorageFactory.h"

class ObjectResourceFactory : public IResourceFactory
{

public:
    ObjectResourceFactory(QObject* parent = nullptr);
    ObjectResourceFactory(IObjectResourceStorageFactory* storageFactory, QObject* parent = nullptr);
    QString getResourceType() const override;

    void setAlternativeStorageFactory(IObjectResourceStorageFactory *newAlternativeStorageFactory);

private:
    resourcePtr createResource(QString token, QString descriptor, QObject *parent) override;
    IObjectResourceStorageFactory* _alternativeStorageFactory = nullptr;
};

#endif // OBJECTRESOURCEFACTORY_H
