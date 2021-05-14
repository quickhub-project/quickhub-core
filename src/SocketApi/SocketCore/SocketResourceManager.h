/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef SOCKETRESOURCEMANAGER_H
#define SOCKETRESOURCEMANAGER_H

#include <QObject>
#include "../SocketCore/IRequestHandler.h"
#include "../SocketCore/IResourceHandlerFactory.h"
#include "IResourceHandler.h"

class SocketResourceManager : public IRequestHandler
{
    Q_OBJECT

public:
    explicit        SocketResourceManager(QObject* parent = nullptr);
                    ~SocketResourceManager();
    void            init(QString storageDirectory);
    bool            handleRequest(QVariantMap message, ISocket* handle);
    QStringList     getSupportedCommands();
    void            registerFactory(IResourceHandlerFactory* factory);

private:
    QMap<QString, IResourceHandlerFactory*>             _handlerFactorys;
    QMap<QString, QMap<QString, IResourceHandler*>* >   _resourceHandler;
    QString                                             _dataStoragePath;
    QStringList                                         _supportedCommands;

private slots:
    void handlerDeleted(QObject* obj);

};

#endif // SOCKETRESOURCEMANAGER_H
