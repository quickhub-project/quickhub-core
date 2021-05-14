/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IOBJECTRESOURCESTORAGEFACTORY_H
#define IOBJECTRESOURCESTORAGEFACTORY_H

#include <QObject>

class IObjectResourceStorage;
class IObjectResourceStorageFactory : public QObject
{
    Q_OBJECT

public:
    explicit IObjectResourceStorageFactory(QObject* parent = nullptr) : QObject(parent){}
    virtual ~IObjectResourceStorageFactory(){}
    virtual IObjectResourceStorage* createInstance(QString qualifiedResourceName, QObject* parent = nullptr) = 0;
};

#endif // IOBJECTRESOURCESTORAGEFACTORY_H
