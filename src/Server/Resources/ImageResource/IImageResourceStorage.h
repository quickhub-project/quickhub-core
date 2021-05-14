/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IIMAGERESOURCESTORAGE_H
#define IIMAGERESOURCESTORAGE_H

#include <QObject>

class IImageResourceStorage : public QObject
{
    Q_OBJECT

public:
    explicit IImageResourceStorage(QObject* parent):QObject(parent){}
    ~IImageResourceStorage(){}

    virtual bool insertImage(QImage image, QVariantMap metadata, QString uid) = 0;
    virtual bool deleteImage(QString uid) = 0;

    virtual QStringList getAllImageIds() = 0;
    virtual QImage      getImage(QString uid) = 0;
    virtual QVariant    getMetadata(QString uid) = 0;
    virtual QVariantMap getAllMetadata() = 0;
};

#endif // IIMAGERESOURCESTORAGE_H
