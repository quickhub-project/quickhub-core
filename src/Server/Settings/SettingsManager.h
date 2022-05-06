/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "SettingsResource.h"
#include "../Resources/ResourceManager/IResourceFactory.h"
#include <QMutex>
#include "qhcore_global.h"

class COREPLUGINSHARED_EXPORT SettingsManager : public IResourceFactory
{
public:
    SettingsManager(QObject* parent = nullptr);
    QString getResourceType() const override;
    virtual QString getDescriptorPrefix() const override;
    static SettingsManager* instance();
    QVariant getSetting(QString topic, QString key, QVariant init);
    QVariantMap getSettings(QString topic, QVariantMap init);

private:
    QSharedPointer<IResource> getOrCreateSettings(QString topic, bool external);
    resourcePtr createResource(QString token, QString descriptor, QObject *parent) override;
    QMap<QString, QWeakPointer<IResource>> _settings;
    QMutex _mutex;
};

#endif // SETTINGSMANAGER_H
