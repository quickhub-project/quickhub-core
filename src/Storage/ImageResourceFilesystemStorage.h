/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IMAGERESOURCEFILESYSTEMSTORAGE_H
#define IMAGERESOURCEFILESYSTEMSTORAGE_H

#include <QObject>
#include <QMap>
#include <QImage>
#include <QVariant>
#include <QFile>

#include "../Server/Resources/ImageResource/IImageResourceStorage.h"

class ImageResourceFilesystemStorage : public IImageResourceStorage
{
    Q_OBJECT

public:
    explicit ImageResourceFilesystemStorage(QString qualifiedResourceName, QObject *parent = nullptr);
    bool insertImage(QImage image, QVariantMap metadata, QString uid) override;
    bool deleteImage(QString uid) override;
    QStringList getAllImageIds() override;
    QVariant getMetadata(QString uid) override;
    QVariantMap getAllMetadata() override;
    virtual QImage getImage(QString uid) override;
    bool load();
    bool save();

private:
    QFile _file;
    QMap<QString, QImage>       _images;
    QMap<QString, QVariant>     _metadata;
    QString                     _resourceName;
    bool                        _contentLoaded = false;


signals:

public slots:
};

#endif // IMAGERESOURCEFILESYSTEMSTORAGE_H
