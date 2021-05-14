/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef OBJECTRESOURCEFILESYSTEMSTORAGE_H
#define OBJECTRESOURCEFILESYSTEMSTORAGE_H

#include <QObject>
#include <QVariant>
#include <QFile>
#include <QFileInfo>

#include "../Server/Resources/ObjectResource/IObjectResourceStorage.h"

class ObjectResourceFilesystemStorage : public IObjectResourceStorage
{

public:
    ObjectResourceFilesystemStorage(QString qualifiedResourceName, QObject* parent = nullptr);

    virtual bool        insertProperty(QString name, QVariant value);
    virtual bool        sync(); // to write unsaved
    virtual bool        setMetadata(QVariant metadata);

    /*------   getter */

    virtual QVariant    getProperty(QString name) const;
    virtual QVariantMap getAllProperties() const;
    virtual QVariant    getMetadata() const;

private:
    void load();
    bool save();
    QFile       _file;
    QString     _qualifiedResourceName;
    QVariantMap _propertyData;
    QVariant    _metadata;
};

#endif // OBJECTRESOURCEFILESYSTEMSTORAGE_H
