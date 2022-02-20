/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef FILESYSTEMPATHS_H
#define FILESYSTEMPATHS_H

#include <QObject>

class FileSystemPaths : public QObject
{
    Q_OBJECT

public:
    FileSystemPaths(QObject* parent = nullptr);
    static FileSystemPaths* instance();

    void setStoragePath(const QString &newStoragePath);
    QString getStoragePath();

    void setConfigPath(const QString &newConfigPath);
    const QString &getConfigPath() const;



private:
    QString                             _storagePath;
    QString                             _configPath;

};

#endif // FILESYSTEMPATHS_H
