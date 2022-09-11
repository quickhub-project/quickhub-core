/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef LISTRESOURCEFACTORY_H
#define LISTRESOURCEFACTORY_H

#include "../ResourceManager/IResourceFactory.h"
#include "IListResourceStorageFactory.h"

class ListResourceFactory : public IResourceFactory
{
    Q_OBJECT

public:
    ListResourceFactory(IListResourceStorageFactory* storageFactory, QObject* parent = nullptr);
    ListResourceFactory(QObject* parent = nullptr);
    QString getResourceType() const override;

    void setAlternativeStorageFactory(IListResourceStorageFactory *newAlternativeStorageFactory);

private:
    resourcePtr createResource(QString token, QString descriptor, QObject *parent) override;
    IListResourceStorageFactory* _alternativeStorageFactory = nullptr;
};

#endif // LISTRESOURCEFACTORY_H
