/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IMAGERESOURCE_H
#define IMAGERESOURCE_H
#include <QImage>
#include "../ResourceManager/IResource.h"
#include "../../Authentication/User.h"
#include "ImageResourceFactory.h"
#include <QReadWriteLock>

class IImageResourceStorage;
class ResourceManager;
class ImageResource : public IResource
{

    Q_OBJECT
public:

    explicit ImageResource(IImageResourceStorage* storage, QObject* parent = nullptr);

    qint64              lastAccess() const override;
    QString const       getResourceType() const override;
    ModificationResult  insert(QImage image, QVariant data, QString id, QString token);
    ModificationResult  deleteImage(QString uid, QString token);

    QStringList         getAllImageIds(QString token);
    QImage              getImage(QString id,  QString token);
    QVariant            getMetaData(QString id);
    QVariantMap         getAllMetadata();

private:
    QReadWriteLock           _lock;
    qint64                   _lastAccess;
    IImageResourceStorage*   _listStorage;
    QVariantMap prepareTemplate(iIdentityPtr user) const;

signals:
    void imageAdded(QString uid);
    void imageRemoved(QString uid);
};

#endif // IMAGERESOURCE_H
