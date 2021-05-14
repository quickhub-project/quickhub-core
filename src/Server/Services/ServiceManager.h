/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include "IService.h"
#include "qhcore_global.h"
#include <QDebug>
#include <QGlobalStatic>


class COREPLUGINSHARED_EXPORT ServiceManager : public QObject
{
    friend class ServiceManagerDerived;
    Q_OBJECT

public:
    explicit                    ServiceManager(QObject *parent = nullptr);
    static ServiceManager*      instance();
    IService*                   service(QString name);
    bool                        registerService(IService* service);
    QMap<QString, IService*>    getServices();

private:
    QMap<QString, IService*>    _serviceMap;


signals:
    void serviceAdded(QString serviceName);

public slots:
};



#endif // SERVICEMANAGER_H
