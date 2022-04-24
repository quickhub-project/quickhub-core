/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "SettingsManager.h"
#include "SettingsResource.h"
#include "../Resources/ObjectResource/IObjectResourceStorageFactory.h"
#include "PluginManager.h"
#include "Storage/ObjectResourceFilesystemStorage.h"

Q_GLOBAL_STATIC(SettingsManager, settingsResourceFactory);
SettingsManager::SettingsManager(QObject* parent) : IResourceFactory(parent)
{
}

QString SettingsManager::getResourceType() const
{
    return "object";
}

QString SettingsManager::getDescriptorPrefix() const
{
    return "settings/";
}

QSharedPointer<IResource> SettingsManager::getOrCreateSettings(QString resourceName)
{
    QMutexLocker locker(&_mutex);
    if(_settings.contains(resourceName)){
        auto resource = _settings[resourceName];
        if(!resource.isNull())
            return resource.toStrongRef();
    }
    qInfo()<<"Create SettingsResource with FS Resource Handler  "<<resourceName;
    auto resource = resourcePtr(new SettingsResource(new ObjectResourceFilesystemStorage(resourceName, nullptr)));
    _settings.insert(resourceName, resource.toWeakRef());
    connect(resource.data(), &IResource::resourceDestroyed, this, [=](QString descriptor)
    {
        QMutexLocker locker(&_mutex);
        _settings.remove(descriptor);
    });
    return resource;
}

SettingsManager *SettingsManager::instance()
{
    return settingsResourceFactory;
}

QVariant SettingsManager::getSetting(QString topic, QString key, QVariant init)
{
    QString resourceID = getResourceID(getDescriptorPrefix()+topic);
    auto settings = qSharedPointerCast<SettingsResource>(getOrCreateSettings(resourceID));
    QVariantMap data = settings->getObjectData();
    if(!data.contains(key))
    {
        settings->setProperty(key, init);
        return init;
    }

    return data.value(key).toMap()["data"];
}

QVariantMap SettingsManager::getSettings(QString topic, QVariantMap init)
{
    QString resourceID = getResourceID(getDescriptorPrefix()+topic);
    auto settings = qSharedPointerCast<SettingsResource>(getOrCreateSettings(resourceID));
    if(settings->getObjectData().isEmpty())
    {
        QMapIterator<QString, QVariant> it(init);
        while(it.hasNext())
        {
            settings->setProperty(it.key(), it.value());
        }
    }

    QMapIterator<QString, QVariant> it(settings->getObjectData());
    QVariantMap data;
    while(it.hasNext())
    {
        data.insert(it.key(), it.value().toMap()["data"]);
    }
    return data;
}

resourcePtr SettingsManager::createResource(QString token, QString descriptor, QObject *parent)
{
    Q_UNUSED(parent);
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
        return resourcePtr();

    QString resourceName = getResourceID(descriptor);
    return getOrCreateSettings(resourceName);
}
