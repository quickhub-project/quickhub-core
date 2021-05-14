/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef ILISTRESOURCESTORAGEFACTORY_H
#define ILISTRESOURCESTORAGEFACTORY_H

#include <QObject>
#include "qhcore_global.h"

class IListResourceStorage;

/*!
    \class IListResourceStorageFactory
    This class is the abstract interface for ListResourceStorageFactories.
    Derive from this class and add an instance to the object pool to
    change the storage behavior of the default ListResource provided by QuickHub.
*/

class COREPLUGINSHARED_EXPORT IListResourceStorageFactory : public QObject
{
    Q_OBJECT

public:
    explicit IListResourceStorageFactory(QObject* parent = nullptr) : QObject(parent){}
    virtual ~IListResourceStorageFactory(){}

    /*!
        \fn IListResourceStorage* IListResourceStorageFactory::createInstance(QString qualifiedResourceName, QObject* parent = nullptr) = 0
        Create a ListResourceStorage instance and return a pointer to it. Set its parent to the given parent.
        QualifiedResourceName indicates from where to load / where to save the data. It's unique for every single resource.
    */
    virtual IListResourceStorage* createInstance(QString qualifiedResourceName, QObject* parent = nullptr) = 0;
};

#endif // ILISTRESOURCESTORAGEFACTORY_H
