/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef APITOKENLISTRESOURCEFACTORY_H
#define APITOKENLISTRESOURCEFACTORY_H

#include <QObject>
#include "../../Resources/ResourceManager/IResourceFactory.h"

class ApiTokenListResourceFactory : public IResourceFactory
{
    Q_OBJECT

public:
    explicit ApiTokenListResourceFactory(QObject* parent = nullptr);

    QString getResourceID(QString descriptor, QString token = "") const override;
    QString getResourceType() const override;
    QString getDescriptorPrefix() const override;

private:
    resourcePtr createResource(QString token, QString descriptor, QObject* parent = nullptr) override;

    resourcePtr _resource;
};

#endif // APITOKENLISTRESOURCEFACTORY_H
