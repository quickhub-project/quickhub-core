/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "ServiceManager.h"
#include <QCoreApplication>


Q_GLOBAL_STATIC(ServiceManager, serviceManager);

ServiceManager::ServiceManager(QObject *parent) : QObject(parent)
{
}


ServiceManager *ServiceManager::instance()
{
    return serviceManager;
}

bool ServiceManager::registerService(IService *service)
{
    QString name = service->getServiceName();
    if(_serviceMap.contains(name))
            return false;

    _serviceMap.insert(name, service);
    Q_EMIT serviceAdded(name);
    qInfo()<<"Service registered: " + name;
    return true;
}

QMap<QString, IService *> ServiceManager::getServices()
{
    return _serviceMap;
}

IService *ServiceManager::service(QString name)
{
    return _serviceMap.value(name, nullptr);
}
