/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef OBJECTRESOURCE_H
#define OBJECTRESOURCE_H

#include <QObject>
#include <QFile>
#include <QVariant>
#include <QReadWriteLock>

#include "../ResourceManager/IResource.h"
#include "../../Authentication/User.h"
#include "ObjectResourceFactory.h"

class IObjectResourceStorage;
class ResourceManager;
class ObjectResource : public IResource
{
    Q_OBJECT

    friend class ObjectResourceFactory;

public:
    explicit ObjectResource(IObjectResourceStorage* storage, QObject *parent = 0);
    ~ObjectResource() override;

    qint64                      lastAccess() const override;
    QString const               getResourceType() const override;
    virtual QVariantMap         getObjectData() const;
    virtual QVariantMap         getMetaData() const;

    /*!
        Sets the value of a given property.
    */
    virtual ModificationResult  setProperty(QString name, const QVariant &value, QString token);
    virtual bool                setFilter(QVariantMap query);

private:
    qint64                  _lastAccess;
    IObjectResourceStorage* _storage;
    mutable QReadWriteLock  _mutex;

signals:
    void propertyChanged(QString property, QVariant data, iIdentityPtr  user);


public slots:
};

#endif // OBJECTRESOURCE_H
