/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "IResource.h"
#include <QJsonDocument>

#include <QDebug>
#include <QDir>
#include <QCoreApplication>


IResource::IResource(QString path, QObject *parent) : QObject(parent),
    _file(path),
    _resourcePath(path)
{
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &IResource::save);
}

IResource::IResource(QObject *parent) : QObject(parent)
{
}

IResource::~IResource()
{
    Q_EMIT resourceDestroyed(_descriptor);
}

void IResource::save()
{
    QFileInfo info(_file);
    QDir dir(info.absolutePath());
    if(!dir.exists())
    {
        dir.mkpath(info.absolutePath());
    }

    if(_file.open(QFile::WriteOnly))
    {
        _file.write(QJsonDocument::fromVariant(getData()).toJson());
        _file.close();
    }
    else
    {
        qDebug()<<"Warning: Could not open file -"<<_file.errorString();
    }
}

void IResource::setDynamicContent(bool enabled)
{
    _dynamicContent = enabled;
}

QVariantMap IResource::load()
{
    if( _file.open(QFile::ReadOnly))
    {
        QVariantMap file =  QJsonDocument::fromJson(_file.readAll()).toVariant().toMap();
        _file.close();
        return file;
    }
    else
    {
        qDebug()<<"Warning: Could not open File:  "<<_resourcePath<<" - "<<_file.errorString();
        return QVariantMap();
    }
}

bool IResource::setResourcePath(QString path)
{
    _file.setFileName(path);
    _resourcePath = path;
    return true;
}

const QString IResource::getResourcePath()
{
    return _resourcePath;
}

const QVariantMap IResource::getData()
{
    return QVariantMap();
}

bool IResource::dynamicContent() const
{
    return _dynamicContent;
}
