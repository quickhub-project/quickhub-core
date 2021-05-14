/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef FILESYSTEMLOADER_H
#define FILESYSTEMLOADER_H

#include <QObject>
#include <QVariant>
#include <QFile>
#include <QJsonDocument>

class FileSystemLoader : public QObject
{
    Q_OBJECT

public:
    explicit        FileSystemLoader(QString path, QObject *parent = nullptr);
    QVariantMap     load();

public slots:
    bool            deleteFile();
    bool            save(QVariantMap data);

private:
    QString         _resourcePath;
    QFile           _file;

signals:

public slots:
};

#endif // FILESYSTEMLOADER_H
