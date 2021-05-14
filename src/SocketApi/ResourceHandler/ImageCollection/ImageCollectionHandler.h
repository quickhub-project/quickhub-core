/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef IMAGECOLLECTIONHANDLER_H
#define IMAGECOLLECTIONHANDLER_H

#include <QObject>
#include "../../SocketCore/IResourceHandler.h"
#include "Server/Resources/ImageResource/ImageResource.h"

class ImageCollectionHandler : public IResourceHandler
{

    Q_OBJECT

public:
    explicit ImageCollectionHandler(QSharedPointer<ImageResource> resource, QObject *parent = nullptr);
     void initHandle(ISocket* handle) override;
     void handleMessage(QVariant message, ISocket* handle) override;

signals:
private:
     QSharedPointer<ImageResource> _resource;

private slots:
     void imageAddedSlot(QString uuid);

public slots:
};

#endif // IMAGECOLLECTIONHANDLER_H
