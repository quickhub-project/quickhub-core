/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef QOBJECTRESOURCE_H
#define QOBJECTRESOURCE_H

#include "ObjectResource.h"
#include <QMetaMethod>
#include <QObject>

class QObjectResource : public ObjectResource
{
    Q_OBJECT

public:
    QObjectResource(QObject* object, QObject* parent = nullptr);
    bool initObject(QObject* object);
    virtual ModificationResult  setProperty(QString name, const QVariant &value, QString token) override;
    virtual QVariantMap         getObjectData() const override;

private:
    QVariantMap                     toVariant(QObject* object) const;
    QObject*                        _object;
    QMetaMethod                     _changedSlot;
    QMap<int, QMetaProperty>        _propertiesByIndex;
    QMap<QString, QMetaProperty>    _propertiesByName;
    QString                         _className;
    bool                            _initialized = false;

private slots:
    void objectPropertyChanged();

};

#endif // QOBJECTRESOURCE_H
