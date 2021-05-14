/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef IRESOURCE_H
#define IRESOURCE_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QVariant>
#include "qhcore_global.h"

class ResourceManager;
class COREPLUGINSHARED_EXPORT IResource : public QObject
{
    Q_OBJECT
    friend class ResourceManager;

public:
    enum ResourceError
    {
        NO_ERROR = 0,
        PERMISSION_DENIED = -1,
        UNKNOWN_ITEM = -2,
        INVALID_PARAMETERS = -3,
        STORAGE_ERROR = -4,
        UNKNOWN_ERROR = -5
    };

    Q_ENUM (ResourceError)

    /*!
        \struct IResource::ModificationResult
        This structure contains all relevant data for a list modification.
        It contains the error code and a copy of the new, modified data object.
        The copy contains the modified object with all meta information like timestamp,
        UUID of the object and owner.
        Use this data to for broadcasting the changes to clients.
    */
    struct ModificationResult
    {
        QVariant data;
        ResourceError error = NO_ERROR;
    };

    explicit                    IResource(QString path, QObject* parent = nullptr);
    explicit                    IResource(QObject* parent = nullptr);
    QVariantMap                 load();
    bool                        setResourcePath(QString path);
    virtual                     ~IResource();
    virtual const QString       getResourcePath();

    /*!
        \fn qint64 IResource::lastAccess() const override
        Returns a unix timestamp of the last user access.
    */
    virtual qint64              lastAccess() const = 0;

    /*!
        \fn virtual bool IResource::getData()
        Returns a dump of the whole resource.
        The base class will use this data to store the content on disk.
    */
    virtual const QVariantMap   getData();

    /*!
        \fn virtual bool IResource::getResourceType()
        Returns the type of this resource.
    */
    virtual const QString       getResourceType() const = 0;

    /*!
        \fn virtual bool IResource::dynamicContent()
        By default, this function returns false.
        \sa IResource::setDynamicContent(bool enabled)
    */
    bool                        dynamicContent() const;


public slots:
    void                        save();

protected:
    QFile                       _file;
    /*!
        \fn IResource::setDynamicContent(bool enabled)
        Set dynamic content to true, to implement a resource which
        provides individual content specificially for one distinct client. The resource manager will not cache this resource.
        It will be instantiated individually for each client.
    */
    void                        setDynamicContent(bool enabled);

private:
    QString                     _resourcePath;
    bool                        _dynamicContent = false;

    //                          this property is set from friend class ResourceManager and will be sent with the destroyed signal
    //                          It is used for internal bookkeeping in ResourceManager.cpp. This is for threadsafetyness.
    QString                     _descriptor;

signals:
    void                        resourceDestroyed(QString descriptor);
};

#endif // IRESOURCE_H
